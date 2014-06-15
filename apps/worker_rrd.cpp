#include "worker_rrd.h"
#include "lm50client.h"

#include <iostream>
#include <iomanip>
#include <locale>
#include <rrd.h>

namespace LM50 {

/**
 * A little helper function to print a POSIX timespec structure as ISO format
 * YYYY-MM-DD HH:MM:ss.nanoseconds
 */
static std::ostream& operator<<( std::ostream& os, const struct timespec& ts ) {
	struct tm t;
	gmtime_r( &(ts.tv_sec), &t );
	assert( 0 <= ts.tv_nsec && ts.tv_nsec < 1000000000 );
	char fc( os.fill( '0' ) );
	std::locale loc( os.imbue( std::locale( "C" ) ) );
	os << '"' << (1900 + t.tm_year) << '-';
	os << std::setw( 2 ) << t.tm_mon << '-';
	os << std::setw( 2 ) << t.tm_mday << ' ';
	os << std::setw( 2 ) << t.tm_hour << ':';
	os << std::setw( 2 ) << t.tm_min << ':';
	os << std::setw( 2 ) << t.tm_sec << '.';
	os << std::setw( 9 ) << ts.tv_nsec << "\";";
	os.fill( fc );
	os.imbue( loc );
	return os;
}

/**
 * C'tor
 * Most important this function allocates an array to cache the channel values
 * within this class. The array's size is not chosen to store values of
 * all possible channels (i.e. 50) but only large enough to store the requested
 * values.
 * 
 * Example: Lets assume the requested channels are 1, 2, 10, 15.
 * Then _chIdx is a vector of the requestet channel indizes, this means
 * _chIdx = { 1, 2, 10, 15 }. _chSize = 4, because this is the number of
 * requested channels. _chValues is an array to hold 4 ChVal values. For example
 * _chValues = { 1000, 42, 11, 0815 }
 * 
 */
WorkerRrd::WorkerRrd( ModeDaemon &parent ) : DaemonWorker( parent ),\
	_pollingPeriod( parent.app().programOptions().rrdPeriod() ),
	_timeBeat(),\
	_chSize( 0 ),\
	_chIdx( parent.app().programOptions().channels() ),\
	_chValues( nullptr ),\
	_timeUpdate() {
	_chSize = _chIdx.size();
	_chValues = new LM50Device::ChVal[ _chSize ];
}

WorkerRrd::~WorkerRrd() {
	delete[] _chValues;
}

/**
 * The main loop of the thread
 */
int WorkerRrd::run() {
	logHeader();
	
	// Get time point of first beat, i.e. the next multiple of _pollingPeriod from now on
	clock_gettime( CLOCK_REALTIME, &_timeBeat );
	_timeBeat.tv_sec = static_cast<time_t>(_timeBeat.tv_sec / _pollingPeriod) * _pollingPeriod;
	_timeBeat.tv_nsec = 0;
	stepBeat();
	
	// Run a "endless" loop until the termination signal is received
	do  {
		sleepUntilBeat();
		if( isCancelled() ) return 0;
		obtainValues();
		logValues();
		updateRRD();
		stepBeat();
	} while( !isCancelled() );
	return 0;
}

/**
 * Locks the device object, requests the device object to update its internal
 * values from the real physical device and copies the values into this 
 * object's own variables _timeUpdate and _chValues. The latter is necessary
 * in order to be able unlock the device object again.
 */
void WorkerRrd::obtainValues() {
	ProgramOptions::ChList::const_iterator i( _chIdx.begin() );
	LM50Device::ChVal* val( _chValues );
	_parent.lockDevice();
	_parent.deviceUpdate();
	_timeUpdate = _parent.deviceLastUpdate();
	for( ; i != _chIdx.end(); (++i,++val) ) {
		*val = _parent.deviceChannel( *i );
	}
	_parent.unlockDevice();
}

/**
 * Advances the object's _timeBeat variable by the smallest multiple of
 * _pollingPeriod such that the new _timeBeat value is located in the future.
 * Normally the step size is supposed to be one times _pollingPeriod. If not
 * and error is logged.
 */
void WorkerRrd::stepBeat() {
	struct timespec timeNow;
	clock_gettime( CLOCK_REALTIME, &timeNow );
	// Increase time beat to next measurement point and compare to current
	// time. Ensure that the next time beat is in the future.
	_timeBeat.tv_sec += _pollingPeriod;
	if( _timeBeat.tv_sec > timeNow.tv_sec ) return;
	
	while( timeNow.tv_sec >= _timeBeat.tv_sec ) {
		std::clog << "Warning: Update step too long for requested polling period. Skipping time point." << std::endl;
		_timeBeat.tv_sec += _pollingPeriod;
	}
	std::clog << "Warning: Update step too long for requested polling period. Skipping time point." << std::endl;
	_timeBeat.tv_sec += _pollingPeriod;
}

/**
 * Blocks until _timeBeat passes by.
 */
void WorkerRrd::sleepUntilBeat() const {
	// Wrap the call to sleep into a loop, because clock_nanosleep might be
	// interrupted by a signal. If that signal does not need any reaction
	// go to sleep again.
	bool verb( _parent.app().programOptions().beVerbose() );
	int ret(1);
	while( ret != 0 && !isCancelled() ) {
		if( verb ) std::cerr << "Go to sleep ..." << std::flush;
		ret = clock_nanosleep( CLOCK_REALTIME, TIMER_ABSTIME, &_timeBeat, NULL );
		if( verb ) std::cerr << " and wake up" << std::endl;
	}
}

/**
 * Write the object's variables _timeUpdate and _chValues into the RRD file.
 */
void WorkerRrd::updateRRD() {
	// Constructs the argument string for rrd_update_r. The pattern is
	// <timestamp>:<value 1>:....:<value N>
	std::ostringstream rrdArg;
	rrdArg.imbue( std::locale( "C" ) );
	rrdArg << (_timeUpdate.tv_nsec < 500000000l ? _timeUpdate.tv_sec : _timeUpdate.tv_sec+1l );
	LM50Device::ChIdx i(0);
	LM50Device::ChVal* val( _chValues );
	for( ; i != _chSize; (++i,++val) ) {
		rrdArg << ':' <<  *val;
	}
	const char* arg( rrdArg.str().c_str() );
	
	// If in verbose debugging mode, output the argument string that was passed to
	// rrd_update_r. I.e. the string with pattern <timestamp>:<value 1>:....:<value N>
	if( _parent.app().programOptions().beVerbose() ) std::cerr << "rrd_update: " << arg << std::endl;
	
	// Do the actual update
	const char* file( _parent.app().programOptions().rrdFile().c_str() );
	rrd_clear_error();
	if( rrd_update_r( file, nullptr, 1, &arg ) ) throw std::runtime_error( rrd_get_error() );
}

/**
 * Write a csv-like header to standard error. Does not do anything, if not in
 * verbose debugging mode
 */
void WorkerRrd::logHeader() const {
	if( !_parent.app().programOptions().stayInForeground() ) return;
	// Print header in CSV file. A 32bit integer has at most 10 digets, hence
	// the column width must be at least 11 characters. But due the caption
	// and the surrounding quotation marks the column width is 12 characters
	// anyway. The time column 31 characters including the quotation marks.
	std::cerr << "\"Time\"                         ;";
	std::cerr << std::setfill( '0' );
	ProgramOptions::ChList::const_iterator i( _chIdx.begin() );
	for( ; i != _chIdx.end(); ++i ) {
		std::cerr << "\"Channel " << std::setw( 2 ) << (*i+1) << "\";";
	}
	std::cerr << std::setfill( ' ' ) << std::endl;
}

/**
 * Write a csv-like line to standard error. Does not do anything, if not in
 * verbose debugging mode
 */
void WorkerRrd::logValues() const {
	if( !_parent.app().programOptions().stayInForeground() ) return;
	LM50Device::ChIdx i(0);
	LM50Device::ChVal* val( _chValues );
	std::cerr << _timeUpdate;
	for( ; i != _chSize; (++i,++val) ) {
		std::cerr << std::setw( 12 ) << *val << ';';
	}
	std::cerr << std::endl;
}

}

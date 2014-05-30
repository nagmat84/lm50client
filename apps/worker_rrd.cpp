#include <fstream>
#include "worker_rrd.h"
#include "lm50client.h"

namespace LM50 {

WorkerRrd::WorkerRrd( ModeDaemon &parent ) : DaemonWorker( parent ) {
}

int WorkerRrd::run() {
	std::ofstream file;
	file.open( "/tmp/lm50client.csv" );
	file.imbue( std::locale( "C" ) );
	
	unsigned long pollingPeriod( _parent.app().programOptions().rrdPeriod() );
	struct timespec timeNow;
	struct timespec timeBeat;
	int ret(1);
	
	const ProgramOptions::ChList& ch( _parent.app().programOptions().channels() );
	ProgramOptions::ChList::const_iterator chIt;
	
	bool fg( _parent.app().programOptions().stayInForeground() );
	bool verb( _parent.app().programOptions().beVerbose() );
	
	// Print header in CSV file. A 32bit integer has at most 10 digets, hence
	// the column width must be at least 11 characters. But due the caption
	// and the surrounding quotation marks the column width is 12 characters
	// anyway. The time column 31 characters including the quotation marks.
	file << "\"Time\"                         ;";
	if( fg ) std::cout << "\"Time\"                         ;";
	for( chIt =  ch.begin(); chIt != ch.end(); ++chIt ) {
		file << "\"Channel " << std::setw( 2 ) << std::setfill( '0' ) << (*chIt+1) << "\";";
		if( fg ) std::cout << "\"Channel " << std::setw( 2 ) << std::setfill( '0' ) << (*chIt+1) << "\";";
	}
	file << std::setfill( ' ' ) << std::endl;
	if( fg ) std::cout << std::setfill( ' ' ) << std::endl;
	

	// Get time point of first measurement
	clock_gettime( CLOCK_REALTIME, &timeBeat );
	// Run a "endless" loop until the termination signal is received
	do  {
		// Obtain new values from device and write them to file
		_parent.device().updateVolatileValues();
		file << '"' << std::setw( 29 ) << std::setfill( '0' ) << boost::posix_time::to_simple_string( _parent.device().lastUpdate() ) << "\";";
		if( verb ) std::cout << '"' << std::setw( 29 ) << std::setfill( '0' ) << boost::posix_time::to_simple_string( _parent.device().lastUpdate() ) << "\";";
		for( chIt =  ch.begin(); chIt != ch.end(); ++chIt ) {
			file << std::setw( 12 ) << std::setfill( ' ' ) << _parent.device().channel( *chIt ) << ';';
			if( fg ) std::cout << std::setw( 12 ) << std::setfill( ' ' ) << _parent.device().channel( *chIt ) << ';';
		}
		file << std::endl;
		if( fg ) std::cout << std::endl;
		
		// Increase time beat to next measurement point and compare to current
		// time. Ensure that the next time beat is in the future.
		timeBeat.tv_sec += pollingPeriod;
		clock_gettime( CLOCK_REALTIME, &timeNow );
		while( timeNow.tv_sec > timeBeat.tv_sec ) {
			if( fg ) std::cerr << "Warning: Update step too long for requested polling period. Skipping time point." << std::endl;
			timeBeat.tv_sec += pollingPeriod;
		}
		
		// Wrap the call to sleep into a loop, because clock_nanosleep might be
		// interrupted by a signal. If that signal does not need any reaction
		// go to sleep again.
		ret = 1;
		while( ret != 0 && !isCancelled() ) {
			if( verb ) std::cerr << "Go to sleep ..." << std::flush;
			ret = clock_nanosleep( CLOCK_REALTIME, TIMER_ABSTIME, &timeBeat, NULL );
			if( verb ) std::cerr << " and wake up" << std::endl;
		}
		
	} while( !isCancelled() );
	
	file.close();
	return 0;
}

}

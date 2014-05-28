#include "mode_daemon.h"
#include "lm50client.h"
#include <unistd.h>

namespace LM50 {

ModeDaemon* ModeDaemon::_me( nullptr );

ModeDaemon::ModeDaemon( const LM50ClientApp& app ) : \
	ProgramMode( app ), \
	_dev( app.programOptions().host(), \
	app.programOptions().port() ), \
	_action( NoAction ), \
	_file() {
	sigaddset( &_signalSet, SIGHUP );
	sigaddset( &_signalSet, SIGINT );
	sigaddset( &_signalSet, SIGQUIT );
	sigaddset( &_signalSet, SIGTERM );
	sigaddset( &_signalSet, SIGTSTP );
	sigaddset( &_signalSet, SIGUSR1 );
	sigaddset( &_signalSet, SIGUSR2 );
}

void ModeDaemon::handleSignal( int sigNo ) {
	ProgramMode::handleSignal( sigNo );
	switch( sigNo ) {
		case SIGHUP:
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
		case SIGTSTP:
			_action = Terminate;
			break;
		default:
			_action = NoAction;
			break;
	}
}

/**
 * Forks the process in order to become a daemon if necessary, i.e. if
 * requested by program options.
 * @return False, if program execution is supposed to be terminated, and true,
 * if the program execution is supposed to go on. This means, if the process
 * is actually forked the function returns "false" in the parent process,
 * because the parent process must be terminated, and "true" in the child
 * process, because the child continuous running and does the work. If the
 * process is not forked (because the program options say so), the function
 * return "true", because the original process must continue.s
 */
bool ModeDaemon::daemonize() {
	// If we are supposed to stay in foreground, do nothing and return true
	// to make the caller to go on
	if( _app.programOptions().stayInForeground() ) return true;
	
	pid_t pid = fork();
	if( pid < 0 ) {
		// Fork failed, we are still in parent process. Throw error. Although
		// the return statement is never executed, return false to please the
		// compilers
		throw std::runtime_error( "Could not daemonize" );
		return false;
	}
	
	// Fork suceeded, return false for the parent process
	if( pid != 0 ) return false;
	
	// From now on, this is the child process
	
	// Redirect all standard in- and output to /dev/null. There are three
	// approaches around in web. Some manipulate C++ iostream::rdbuf, some
	// change the C stdout, stderr, stdin FILE* streams and the third is what
	// one sees below, i.e. directly change the file descriptors. We use the
	// latter, because both C++ iostream and C FILE* streams work on top of
	// the file descriptors.
	bool err( false );
	int fd( open( "/dev/null", O_RDWR ) );
	err |= ( fd < 0 );
	err |= ( close(2) < 0 );
	err |= ( close(1) );
	err |= ( close(0) );
	err |= ( dup2( fd, 0 ) < 0 );
	err |= ( dup2( fd, 1 ) < 0 );
	err |= ( dup2( fd, 2 ) < 0 );
	err |= ( close( fd ) < 0   );
	if( err ) {
		throw std::runtime_error( "Could not redirect standard i/o to /dev/null" );
		return false;
	}
	
	// Detach from controlling terminal (detach from tty) and place this process
	// in a fresh process group
	if( setsid() < 0 ) {
		throw std::runtime_error( "Could not create new session for child process" );
		return false;
	}
	
	return true;
}

void ModeDaemon::init() {
	_dev.connect();
	_file.open( "/tmp/lm50client.csv" );
	_file.imbue( std::locale( "C" ) );
	
}

void ModeDaemon::deinit() {
	_file.close();
	_dev.disconnect();
}

void ModeDaemon::run() {
	init();
	if( !daemonize() ) return;
	
	//const long nsPerTick( 1000000000 / boost::posix_time::time_duration::ticks_per_second() );
	//boost::posix_time::ptime startUpdate( boost::posix_time::not_a_date_time );
	//boost::posix_time::ptime stopUpdate( boost::posix_time::not_a_date_time );
	unsigned long pollingPeriod( _app.programOptions().pollingPeriod() );
	//boost::posix_time::time_duration sleepDuration( pollingPeriod );
	struct timespec timeNow;
	struct timespec timeBeat;
	int ret(1);
	
	const ProgramOptions::ChList& ch( _app.programOptions().channels() );
	ProgramOptions::ChList::const_iterator chIt;
	
	bool fg( _app.programOptions().stayInForeground() );
	bool verb( _app.programOptions().beVerbose() );
	
	// Print header in CSV file. A 32bit integer has at most 10 digets, hence
	// the column width must be at least 11 characters. But due the caption
	// and the surrounding quotation marks the column width is 12 characters
	// anyway. The time column 31 characters including the quotation marks.
	_file << "\"Time\"                         ;";
	if( fg ) std::cout << "\"Time\"                         ;";
	for( chIt =  ch.begin(); chIt != ch.end(); ++chIt ) {
		_file << "\"Channel " << std::setw( 2 ) << std::setfill( '0' ) << (*chIt+1) << "\";";
		if( fg ) std::cout << "\"Channel " << std::setw( 2 ) << std::setfill( '0' ) << (*chIt+1) << "\";";
	}
	_file << std::setfill( ' ' ) << std::endl;
	if( fg ) std::cout << std::setfill( ' ' ) << std::endl;
	

	// Get time point of first measurement
	clock_gettime( CLOCK_REALTIME, &timeBeat );
	// Run a "endless" loop until the termination signal is received
	do  {
		// Obtain new values from device and write them to file
		_dev.updateVolatileValues();
		_file << '"' << std::setw( 29 ) << std::setfill( '0' ) << boost::posix_time::to_simple_string( _dev.lastUpdate() ) << "\";";
		if( verb ) std::cout << '"' << std::setw( 29 ) << std::setfill( '0' ) << boost::posix_time::to_simple_string( _dev.lastUpdate() ) << "\";";
		for( chIt =  ch.begin(); chIt != ch.end(); ++chIt ) {
			_file << std::setw( 12 ) << std::setfill( ' ' ) << _dev.channel( *chIt ) << ';';
			if( fg ) std::cout << std::setw( 12 ) << std::setfill( ' ' ) << _dev.channel( *chIt ) << ';';
		}
		_file << std::endl;
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
		while( ret != 0 && _action == NoAction ) {
			if( verb ) std::cerr << "Go to sleep ..." << std::flush;
			ret = clock_nanosleep( CLOCK_REALTIME, TIMER_ABSTIME, &timeBeat, NULL );
			if( verb ) std::cerr << " and wake up" << std::endl;
		}
		
	} while( _action != Terminate );
	
	deinit();
}

}

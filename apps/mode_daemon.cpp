#include "mode_daemon.h"
#include "lm50client.h"
#include "worker_rrd.h"
#include <unistd.h>

namespace LM50 {

ModeDaemon::ModeDaemon( const LM50ClientApp& app ) : \
	ProgramMode( app ), \
	_isCancelled( true ), \
	_dev( app.programOptions().host(), app.programOptions().port() ), \
	_mutex(), \
	_mutex_attr(), \
	_mutex_owner() {
	assert( pthread_mutexattr_init( &_mutex_attr ) == 0 );
	assert( pthread_mutexattr_setprotocol( &_mutex_attr, PTHREAD_PRIO_INHERIT ) == 0 );
	assert( pthread_mutexattr_settype( &_mutex_attr, PTHREAD_MUTEX_RECURSIVE ) == 0 );
	assert( pthread_mutex_init( &_mutex, &_mutex_attr ) == 0 );
}

ModeDaemon::~ModeDaemon() {
	assert( pthread_mutex_destroy( &_mutex ) == 0 );
	assert( pthread_mutexattr_destroy( &_mutex_attr ) == 0 );
}

void ModeDaemon::lockDevice() {
	pthread_mutex_lock( &_mutex );
#ifdef DEBUG
	_mutex_owner = pthread_self();
#endif
}

void ModeDaemon::unlockDevice() {
#ifdef DEBUG
	assert( pthread_equal( pthread_self(), _mutex_owner ) );
	_mutex_owner = pthread_t();
#endif
	pthread_mutex_unlock( &_mutex );
}

/**
 * Requests the device object to update its internal attributes with the
 * values from the real physical device.
 * 
 * If a runtime_error is catched this is most likely due a time out. Probabbly
 * the physical device became unavailable. This function tries to reconnect
 * in an endless loop but keeps the device mutex locked. Please note:
 * 
 * (1) This function is executed in the context of the calling thread. Hence,
 *     the main thread is still actively waiting for an termination signal
 *     at ModeDaemon::run() by sigwait. Though this function runs "endlessly"
 *     until the device is reconnected a proper termination is still being
 *     guaranteed.
 * (2) As the mutex is kept locked any child thread that tries to access the
 *     device will go to sleep eventually. This make sense because these other
 *     child threads cannot do anything anyway, if the device is not available.
 *     As soon as the device becomes available again, the calling thread of
 *     this function will continue, unlock the mutex and all other child threads
 *     will be waked up again, too.
 */
void ModeDaemon::deviceUpdate() {
#ifdef DEBUG
	assert( pthread_equal( pthread_self(), _mutex_owner ) );
#endif
	bool verb( _app.programOptions().beVerbose() );
	
	// Try "endlessly" until main thread is cancelled
	while( !isCancelled() ) {
		try {
			_dev.updateVolatileValues();
			return; // if the line above did not throw an exception, it's done
		} catch( std::runtime_error& e ) {
			if( verb ) std::cerr << "Connection to device lost" << std::endl;
			// If update failed, the device probably became unavailable. Disconnect
			// first to get back into a clean state
			_dev.disconnect();
			// Try "endlessly" to connect again until main thread is cancelled
			while( !isCancelled() ) {
				try {
					if( verb ) std::cerr << "Try to reconnect ... " << std::flush;
					_dev.connect();
					if( verb ) std::cerr << "success" << std::endl;
					break; // if the line above did not throw an exception, connection is re-established
				} catch( std::runtime_error& e ) {
					if( verb ) std::cerr << "failed" << std::endl;
				}
			}
		}
	}
}

const struct timespec& ModeDaemon::deviceLastUpdate() {
#ifdef DEBUG
	assert( pthread_equal( pthread_self(), _mutex_owner ) );
#endif
	return _dev.lastUpdate();
}

unsigned int ModeDaemon::deviceChannel( LM50Device::ChIdx ch ) {
#ifdef DEBUG
	assert( pthread_equal( pthread_self(), _mutex_owner ) );
#endif
	return _dev.channel( ch );
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
}

void ModeDaemon::deinit() {
	_dev.disconnect();
}

void ModeDaemon::run() {
	init();
	if( !daemonize() ) return;
	_isCancelled = false;
	
	// Ensure that all signals are blocked such that this threat can synchronously
	// wait for any termination signal later.
	sigset_t allSig;
	sigfillset( &allSig );
	int err( pthread_sigmask( SIG_SETMASK, &allSig, NULL ) );
	if( err ) return throw std::runtime_error( "Could not block signals" );
	
	// Start the worker threads that are requested by program options
	WorkerRrd workerRrd( *this );
	if( _app.programOptions().rrdEnabled() ) workerRrd.start();
	
	// Synchronously wait for any termination related signal
	sigset_t termSig;
	sigemptyset( &termSig );
	sigaddset( &termSig, SIGHUP );
	sigaddset( &termSig, SIGINT );
	sigaddset( &termSig, SIGQUIT );
	sigaddset( &termSig, SIGTERM );
	sigaddset( &termSig, SIGTSTP );
	int sigNo(0);
	err = sigwait( &termSig, &sigNo );
	
	// After a signal arrived terminate enabled workers and deinit.
	_isCancelled = true;
	if( _app.programOptions().rrdEnabled() ) workerRrd.terminate();
	deinit();
	
	// As a final step, check if the arrived signal was really a termination
	// signal or if something else went wrong
	if(err) throw std::runtime_error( "Could not wait for signals" );
	if( sigNo != SIGHUP && sigNo != SIGINT && sigNo != SIGQUIT && sigNo != SIGTERM && sigNo != SIGTSTP )
		throw std::runtime_error( "Unexpected signal arrived" );
}

}

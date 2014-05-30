#include "daemon_worker.h"

namespace LM50 {

DaemonWorker::DaemonWorker( ModeDaemon &parent ) :\
	_parent( parent ),\
	_isRunning( false ),\
	_isCancelled( true ),\
	_thread( 0 ),\
	_thread_attr() {
	assert( pthread_attr_init( &_thread_attr ) == 0 );
	assert( pthread_attr_setdetachstate( &_thread_attr, PTHREAD_CREATE_JOINABLE ) == 0 );
}

DaemonWorker::~DaemonWorker() {
	assert( _isCancelled && !_isRunning );
	pthread_attr_destroy( &_thread_attr );
}

void DaemonWorker::start() {
	if( _isRunning ) throw std::runtime_error( "Thread is already running" );
	_isCancelled = false; // Set to false in case it is true from previous runs
	int res( pthread_create( &_thread, &_thread_attr, main, this ) );
	switch( res ) {
		case EAGAIN:
			throw std::runtime_error( "Insufficient system ressources to start a thread" );
			break;
		case EINVAL:
			// This error means that "_thread_attr" is not a valid. Must nor happen
			assert( false );
			break;
		case EPERM:
			throw std::runtime_error( "Insufficient rights to start a thread" );
			break;
		case 0:
			break;
		default:
			assert( false );
			break;
	}
}

int DaemonWorker::terminate() {
	if( !_isRunning ) throw std::runtime_error( "Thread is not running" );
	_isCancelled = true;
	
	// Send an arbitrary signal to the thread, such that it returns from any
	// blocking function the thread might be in.
	// The error ESRCH means that "_thread" is not valid. Might happen if the
	// thread has already terminated. Ignore silently.
	int res( pthread_kill( _thread, SIGRTMAX ) );
	assert( res == 0 || res == ESRCH );
	
	// Join the thread
	int status( -1 );
	if( pthread_join( _thread, (void**)&status ) ) throw std::runtime_error( "Could not wait for thread to join" );
	_thread = 0;
	_isRunning = false;
	return status;
}

void* DaemonWorker::main( void *me ) {
	// This function set _isRunning to true, because we are inside the thread's
	// context for the first time, but does not set _isRunning back to false
	// before return. The latter is done by terminate() outside the thread's
	// context.
	DaemonWorker *self( static_cast<DaemonWorker*>( me ) );
	self->_isRunning = true;
	
	// Ensure that all signals but SIGRTMAX are blocked for this thread. The
	// special signal SIGRTMAX is used to indicate termination.
	sigset_t mask;
	sigfillset( &mask );
	sigdelset( &mask, SIGRTMAX );
	int err( pthread_sigmask( SIG_SETMASK, &mask, NULL ) );
	if( err ) return reinterpret_cast<void*>( err );
	
	return reinterpret_cast<void*>( self->run() );
}

}

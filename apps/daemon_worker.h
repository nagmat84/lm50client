#ifndef _DAEMON_WORKER_H_
#define _DAEMON_WORKER_H_

#include <pthread.h>
#include "mode_daemon.h"

namespace LM50 {

class DaemonWorker {
	public:
		DaemonWorker( ModeDaemon &parent );
		virtual ~DaemonWorker();
		
	public:
		/**
		 * Create a new joinable thread that executes run(). As run() is executed
		 * in an new thread, start() returns independently of run()
		 */
		void start();
		
		/**
		 * Requests the thread to terminate. This function blocks until then.
		 * Internally the function performs three main steps:
		 * a) It sets _isCancelled to true such that the thread's main function
		 *    will break on the next check
		 * b) It sends a signal to the thread in case the thread's main function
		 *    is blocked by a function that is a cancellation point such that
		 *    the thread's main function gets the capability to check for
		 *    cancellation
		 * c) It joins the thread
		 */
		int terminate();
		
		bool isRunning() const { return _isRunning; }
		
		bool isCancelled() const { return _isCancelled; }
		
		const ModeDaemon& parent() const { return _parent; }
		
	protected:
		/**
		 * The worker's main loop that runs in its own thread context. Must be
		 * overriden by child classes. If this function exits the thread
		 * therminates. The implementation of this function must periodically check
		 * isCancelled and return if true
		 */
		virtual int run() = 0;
		
	private:
		static void* main( void *me );
		
	protected:
		ModeDaemon &_parent;
		
	private:
		/**
		 * False by default. True while thread is running.
		 */
		bool _isRunning;
		
		/**
		 * True by default. Set to false by start(). N.b.: This is not the
		 * direct opposite of _isRunning. A thread can still be running though
		 * already being cancelled during it's end of life while it is on the verge
		 * of terminating.
		 */
		bool _isCancelled;
		
		/**
		 * The thread in whose context the run() is executed
		 */
		pthread_t _thread;
		
		/**
		 * This attribute is used to create the thread. The thread is created as
		 * "joinable" which means that the thread has to be waited for after
		 * terminating
		 */
		pthread_attr_t _thread_attr;
};

}

#endif
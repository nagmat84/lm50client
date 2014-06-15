#ifndef _MODE_DAEMON_H_
#define _MODE_DAEMON_H_

#include "program_mode.h"
#include "core/lm50device.h"

namespace LM50 {

class ModeDaemon : public ProgramMode {
	public:
		ModeDaemon( const LM50ClientApp& app );
		virtual ~ModeDaemon();
		
	public:
		virtual void run();
		
		/**
		 * Locks the mutex that protects the device object from simultanuous access
		 * by different threads. This function must be called prior to any
		 * deviceXY function. The function blocks the calling thread until the
		 * mutex can be aquired.
		 */
		void lockDevice();
		
		/**
		 * Unlock the mutex that protects the device object from simultanuous access
		 * by different threads.
		 */
		void unlockDevice();
		
		/**
		 * Updates the volatile values of the device. N.b.: There is no function
		 * that directly returns the device object, because all function calls to
		 * the device must be secured by a mutex
		 */
		void deviceUpdate();
		
		/**
		 * Gets the time of the last update of the device's values. N.b.: There is
		 * no function that directly returns the device object, because all function
		 * calls to the device must be secured by a mutex. For the same reason
		 * this function returns a deep copy.
		 */
		const struct timespec& deviceLastUpdate();
		
		/**
		 * Returns the value of the device's channel. N.b.: There is no function
		 * that directly returns the device object, because all function calls to
		 * the device must be secured by a mutex
		 */
		unsigned int deviceChannel( LM50Device::ChIdx ch );
		
		
		bool isCancelled() const { return _isCancelled; }
		
	protected:
		bool daemonize();
		void init();
		void deinit();
		
	protected:
		/**
		 * True by default. Set to false by start(). N.b.: This is not the
		 * direct opposite of _isRunning. A thread can still be running though
		 * already being cancelled during it's end of life while it is on the verge
		 * of terminating.
		 */
		bool _isCancelled;
		
		LM50Device _dev;
		
		/**
		 * This mutex is used to protect against parallel access to _dev
		 */
		pthread_mutex_t _mutex;
		
		/**
		 * This attribute is used to create _mutex. This mutex is
		 * creates as recursive, stalling and with priority inheritance.
		 *
		 * A recursive mutex can be locked more than one times by the same thread
		 * without blocking. An internal counter is increased/decreased for each
		 * lock/unlock operation. When the counter drops to zero again, the mutex can be
		 * aquired by another thread.
		 *
		 * A stalling mutex (opposed to a robust mutex) is not unlocked, if the
		 * owning thread dies abnormally. This means that waiting threads are blocked
		 * eternally.
		 * 
		 * Priority inheritence means the scheduling priority of the thread that
		 * owns the mutex is increased to the highest priority of all threads that
		 * are waiting for the mutex to become free. 
		 */
		pthread_mutexattr_t _mutex_attr;
		
#ifdef DEBUG
		/**
		 * This variable holds the thread id that has currently locked _mutex.
		 * Unfortunately there is no library function to obtain the holder of a
		 * mutex, hence this information must be stored seperately.
		 */
		pthread_t _mutex_owner;
#endif
};

}

#endif

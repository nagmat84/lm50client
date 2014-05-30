#ifndef _MODE_DAEMON_H_
#define _MODE_DAEMON_H_

#include "program_mode.h"
#include "core/lm50device.h"

namespace LM50 {

class ModeDaemon : public ProgramMode {
	public:
		ModeDaemon( const LM50ClientApp& app );
		virtual ~ModeDaemon() {}
		
	public:
		virtual void run();
		
		// TODO: Must be protected by mutexes
		LM50Device& device() { return _dev; }
		
	protected:
		bool daemonize();
		void init();
		void deinit();
		
	protected:
		LM50Device _dev;
};

}

#endif

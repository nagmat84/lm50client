#ifndef _MODE_DAEMON_H_
#define _MODE_DAEMON_H_

#include <fstream>
#include "program_mode.h"
#include "core/lm50device.h"

namespace LM50 {

class ModeDaemon : public ProgramMode {
	enum Action { NoAction, Terminate };
	
	public:
		ModeDaemon( const LM50ClientApp& app );
		virtual ~ModeDaemon() {}
		
	public:
		virtual void handleSignal( int sigNo );
		virtual void run();
		
	protected:
		bool daemonize();
		void init();
		void deinit();
		
	protected:
		static ModeDaemon *_me;
		LM50Device _dev;
		Action _action;
		std::ofstream _file;
};

}

#endif

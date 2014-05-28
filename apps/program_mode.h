#ifndef _PROGRAM_MODE_H_
#define _PROGRAM_MODE_H_

#include <signal.h>

namespace LM50 {

class LM50ClientApp;

class ProgramMode {
	public:
		ProgramMode( const LM50ClientApp& app ) : _app( app ), _signalSet() { sigemptyset( &_signalSet ); }
		virtual ~ProgramMode() {}
		
	public:
		const sigset_t& signalSet() const { return _signalSet; }
		virtual void run() = 0;
		virtual void handleSignal( int ) {}
		
	protected:
		const LM50ClientApp& _app;
		sigset_t _signalSet;
};

}

#endif

#ifndef _PROGRAM_MODE_H_
#define _PROGRAM_MODE_H_

namespace LM50 {

class LM50ClientApp;

class ProgramMode {
	public:
		ProgramMode( const LM50ClientApp& app ) : _app( app ) {}
		virtual ~ProgramMode() {}
		
	public:
		virtual void run() = 0;
		
	protected:
		const LM50ClientApp& _app;
};

}

#endif

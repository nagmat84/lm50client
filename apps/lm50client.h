#ifndef _LM50CLIENT_H_
#define _LM50CLIENT_H_

#include "program_options.h"
#include "program_mode.h"

namespace LM50 {

class LM50ClientApp {
	private:
		LM50ClientApp( const ProgramOptions& po );
		LM50ClientApp( const LM50ClientApp& ) { assert( false ); }
		LM50ClientApp& operator=( const LM50ClientApp& ) { assert( false ); return *this; }
		virtual ~LM50ClientApp();
		
	public:
		static void handleSignal( int sigNo );
		static LM50ClientApp& create( const ProgramOptions& po );
		static LM50ClientApp& me() { return *_me; }
		void destroy() const;
		void init();
		void run();
		const ProgramOptions& programOptions() const { return _pOptions; }
		
	protected:
		static LM50ClientApp *_me;
		ProgramMode   *_pMode;
		ProgramOptions _pOptions;
};

}

#endif

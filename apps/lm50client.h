#ifndef _LM50CLIENT_H_
#define _LM50CLIENT_H_

#include "program_options.h"

namespace LM50 {

class LM50ClientApp {
	public:
		LM50ClientApp( const ProgramOptions& po );
		virtual ~LM50ClientApp() {}
		
	public:
		void init();
		void run();
		const ProgramOptions& programOptions() const { return _programOptions; }
		
	protected:
		ProgramOptions _programOptions;
};

}

#endif

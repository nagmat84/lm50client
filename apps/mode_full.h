#ifndef _MODE_FULL_H_
#define _MODE_FULL_H_

#include "program_mode.h"

namespace LM50 {

class ModeFull : public ProgramMode {
	public:
		ModeFull( const LM50ClientApp& app ) : ProgramMode( app ) {}
		virtual ~ModeFull() {}
		
	public:
		void run();
};

}

#endif

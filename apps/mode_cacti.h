#ifndef _MODE_CACTI_H_
#define _MODE_CACTI_H_

#include "program_mode.h"

namespace LM50 {

class ModeCacti : public ProgramMode {
	public:
		ModeCacti( const LM50ClientApp& app ) : ProgramMode( app ) {}
		virtual ~ModeCacti() {}
		
	public:
		void run();
};

}

#endif

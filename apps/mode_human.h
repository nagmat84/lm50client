#ifndef _MODE_HUMAN_H_
#define _MODE_HUMAN_H_

#include "program_mode.h"

namespace LM50 {

class ModeHuman : public ProgramMode {
	public:
		ModeHuman( const LM50ClientApp& app ) : ProgramMode( app ) {}
		virtual ~ModeHuman() {}
		
	public:
		virtual void run();
};

}

#endif

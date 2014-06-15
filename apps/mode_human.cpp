#include "mode_human.h"
#include "lm50client.h"
#include "core/lm50device.h"

#include <iostream>
#include <iomanip>

namespace LM50 {

/**
 * Queries everything that might be useful from the device and prints the
 * received data to standard output in a nice human readable form.
 * Errors and their description are printed to standard output, too.
 */
void ModeHuman::run() {
	std::cout << "===  LM-50TCP+ === " << std::endl << std::endl;
	std::cout << "Host    :  " << _app.programOptions().host() << std::endl;
	std::cout << "Port    :  " << _app.programOptions().port() << std::endl;
	
	LM50Device dev( _app.programOptions().host(), _app.programOptions().port() );
	dev.connect();
	
	dev.readSteadyValues();
	std::cout << "Version :  " << dev.revision() << std::endl;
	std::cout << "Serial  :  " << dev.serialNumber() << std::endl;
	
	std::cout << "Reading channels ... " << std::flush;
	dev.updateVolatileValues();
	std::cout << "done" << std::endl;
	
	for( ProgramOptions::ChList::const_iterator it( _app.programOptions().channels().begin() ); it != _app.programOptions().channels().end(); it++ ) {
		std::cout << "Channel " << std::setw( 2 ) << (*it+1) << ":   " << std::setw(12) << dev.channel(*it) << std::endl;
	}
	
	dev.disconnect();
}

}

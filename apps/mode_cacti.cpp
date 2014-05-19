#include "mode_cacti.h"
#include "lm50client.h"
#include "core/lm50device.h"

namespace LM50 {

/**
 * Queries those input registers that are specified in the command line options
 * and prints their values to standard output such that they can be used by
 * Cacti.
 */
void ModeCacti::run() {
	// Save old locale and set locale to C locale
	std::ostringstream cactiOutput;
	cactiOutput.imbue( std::locale( "C" ) );
	cactiOutput.fill( '0' );
	
	LM50Device dev( _app.programOptions().host(), _app.programOptions().port() );
	dev.connect();
	dev.updateVolatileValues();
	dev.disconnect();
	
	// Cacti threats single value output and multi value output differently.
	// If only one value is expected, only that value must be printed.
	// If more than one value is expected, the values must be preceded by a
	// name and a colon; diffent name/value pairs must be seperated by one
	// whitespace. E.g.
	//
	//   Single value output:   4711
	//   Multi value output:    meter01:4711 meter02:42 meter03:0815
	//
	// Because Catci uses interprocess communication it is very sensible
	// about how the output is written. The full output must be written
	// at once, otherwise it might happen, that Cacti only receives the
	// first part. And the only spaces in the output must be the single
	// spaces between the a value and the following field identifier.
	// If there are more spaces in between or trailing spaces at the end
	// the string is split in a false way.
	
	if( _app.programOptions().channels().size() == 1 ) {
		cactiOutput << dev.channel( _app.programOptions().channels().front() );
	} else {
		ProgramOptions::ChList::const_iterator it( _app.programOptions().channels().begin() );
		cactiOutput << "meter" << std::setw( 2 ) << (*it+1) << ':' << dev.channel( *it );
		for( ++it; it != _app.programOptions().channels().end(); it++ ) {
			cactiOutput<< " meter" << std::setw( 2 ) << (*it+1) << ':' << dev.channel( *it );
		}
	}
	std::cout << cactiOutput.str() << std::endl;
}

}

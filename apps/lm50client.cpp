#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include "lm50client.h"
#include "mode_human.h"
#include "mode_cacti.h"

namespace LM50 {

LM50ClientApp::LM50ClientApp( const ProgramOptions& po ) : _programOptions( po ) {
}

void LM50ClientApp::init() {
	std::locale loc( "" );
	std::locale::global( loc );
	std::cout.imbue( loc );
	std::cerr.imbue( loc );
	std::clog.imbue( loc );
	std::cin.imbue( loc );
}

void LM50ClientApp::run() {
	if ( _programOptions.hasOptionHelp() ) {
		_programOptions.print( std::cout );
		return;
	}
	
	ProgramMode *pmode( nullptr );
	
	switch( _programOptions.operationMode() ) {
		case ProgramOptions::HUMAN:
			pmode = new ModeHuman( *this );
			break;
		case ProgramOptions::CACTI:
			pmode = new ModeCacti( *this );
			break;
		default:
			throw std::runtime_error( "Unknown fatal error" );;
	}
	
	pmode->run();
}

}

int main( int argc, char* argv[] ) {
	try {
		LM50::ProgramOptions po;
		po.parse( argc, argv );
		LM50::LM50ClientApp app( po );
		app.init();
		app.run();
	} catch ( std::exception& e ) {
		std::cerr << "Error: " << e.what() << std::endl;
		return -1;
	} catch ( boost::system::error_code& e ) {
		std::cerr << "Error: " << e.message() << std::endl;
		return e.value();
	} catch ( ... ) {
		std::cerr << "Unknown exception thrown" << std::endl;
		return -1;
	}
	
	return 0;
}


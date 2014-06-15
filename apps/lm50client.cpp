#include "lm50client.h"
#include "mode_human.h"
#include "mode_cacti.h"
#include "mode_daemon.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <clocale>

namespace LM50 {

LM50ClientApp* LM50ClientApp::_me( nullptr );

LM50ClientApp::LM50ClientApp( const ProgramOptions& po ) : \
	_pMode( nullptr ),\
	_pOptions( po ) {
	assert( !_me );
	_me = this;
}

LM50ClientApp::~LM50ClientApp() {
	delete _pMode;
	_me = nullptr;
}


LM50ClientApp& LM50ClientApp::create( const ProgramOptions& po ) {
	if( _me ) throw std::runtime_error( "Application object has already been created" );
	_me = new LM50ClientApp( po );
	return *_me;
}

void LM50ClientApp::destroy() const {
	assert( _me == this );
	delete _me;
	_me = nullptr;
}

void LM50ClientApp::init() {
	setlocale( LC_ALL, "" ); // old C locale
	std::locale loc( "" );
	std::locale::global( loc );
	std::cout.imbue( loc );
	std::cerr.imbue( loc );
	std::clog.imbue( loc );
	std::cin.imbue( loc );
}

void LM50ClientApp::run() {
	if ( _pOptions.hasOptionHelp() ) {
		_pOptions.print( std::cout );
		return;
	}
	
	switch( _pOptions.operationMode() ) {
		case ProgramOptions::HUMAN:
			_pMode = new ModeHuman( *this );
			break;
		case ProgramOptions::CACTI:
			_pMode = new ModeCacti( *this );
			break;
		case ProgramOptions::DAEMON:
			_pMode = new ModeDaemon( *this );
			break;
		default:
			throw std::runtime_error( "Unknown fatal error" );;
	}
	
	// Run the program
	_pMode->run();
}

}

int main( int argc, char* argv[] ) {
	try {
		LM50::ProgramOptions po;
		po.parse( argc, argv );
		LM50::LM50ClientApp& app( LM50::LM50ClientApp::create( po ) );
		app.init();
		app.run();
		app.destroy();
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

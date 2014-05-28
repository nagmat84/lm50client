#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include "lm50client.h"
#include "mode_human.h"
#include "mode_cacti.h"
#include "mode_daemon.h"

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


void LM50ClientApp::handleSignal( int sigNo ) {
	if( !_me ) throw std::runtime_error( "Application caught a signal but it seems that the application does not even exist. What the hell?!" );
	if( _me->_pMode ) _me->_pMode->handleSignal( sigNo );
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
	
	// Install the handler for all signals the that the program mode requires
	// This code is horribly inefficient, because according to POSIX standard
	// there are no limits for the lowest and highest signal nor are there
	// functions to install a handler for all signals in a set at once.
	// (N.b.: There a are some GNU extensions to handle these situations, but
	// their use is not recommended)
	// Hence, the only portable code is to check individually for each signal
	// by "sigismember" and run across all integers. >:O :-( :@
	struct sigaction signalReaction;
	signalReaction.sa_handler = LM50::LM50ClientApp::handleSignal;
	signalReaction.sa_mask = _pMode->signalSet();
	signalReaction.sa_flags = 0;
	for( int s( 0 ); s != _NSIG; ++s ) {
		if( sigismember( &(signalReaction.sa_mask), s) ) sigaction( s, &signalReaction, NULL );
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

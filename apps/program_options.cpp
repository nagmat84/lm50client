#include "program_options.h"

namespace LM50 {

ProgramOptions::ProgramOptions() : \
	_boostOptDesc( "Allowed options" ),\
	_boostVMap(),\
	_operationMode( UNKNOWN ),\
	_hasOptionHelp( false ),\
	_host(),\
	_port(),\
	_channels() {
	_boostOptDesc.add_options()
		( "help", "Prints this help message." )
		( "host,h", boost::program_options::value< std::string >(), "The DNS name of the LM50TCP+ to connect to." )
		( "port,p", boost::program_options::value< std::string >()->default_value( std::string( "502") ), "The port on that the LM50TCP+ listens (default 502). The port can either be given as an integer or as a well-known servive name. E.g. \"http\" is identical to \"80\"." )
		( "full,f", "Operation mode \"full\": Writes results to standard output in a nice human readable layout." )
		( "cacti,c", "Operation mode \"cacti\": Write results to standard output such that they can be parsed by cacti." )
		( "daemon,d", "Operation mode \"deamon\": Forks into background and polls the LM50TCP+ periodically." )
		( "channels,C", boost::program_options::value< ChList >()->multitoken(), "Specifies the channels whose values are polled and processed. Multiple channel numbers must be seperated by white spaces. If the option is specified more than once, the lists of channels are joined. The channels are sorted increasingly and duplicates are skipped. E.g. \"-C 6 11 9 6 -C 11\" is equivalent to \"-C 6 9 11\". If no channels are given, all available channels are polled." );
}

/**
 * @throws std::exception& If any parser error occurs or if some mandatory
 * argument is missing
 * @return True, if "--help" was specified as an argument. In this case the
 * parseOptions( int, char*[] ) prints the help message. The caller is supposed
 * to termintate program execution or do whatever the caller thinks is
 * appropriate. In all other cases the return value is false.
 */
void ProgramOptions::parse( int argCount, char* argVals[] )  {
	boost::program_options::store( boost::program_options::command_line_parser( argCount, argVals ).options( _boostOptDesc ).run(), _boostVMap );
	boost::program_options::notify( _boostVMap );
	
	if( _boostVMap.count( "help" ) ) {
		_hasOptionHelp = true;
		return;
	}
	
	if( _boostVMap.count( "host" ) == 1 ) {
		_host = _boostVMap[ "host" ].as< std::string >();
	} else {
		throw std::logic_error( "The host name must be specified" );
	}
	
	if( _boostVMap.count( "port" ) == 1 ) {
		_port = _boostVMap[ "port" ].as< std::string >();
	} else {
		throw std::logic_error( "The port must be specified" );
	}
	
	// Exactly one of the operation modes must be defined
	if( _boostVMap.count( "full" ) != 0 ) operationMode( FULL );
	if( _boostVMap.count( "cacti" ) != 0 ) operationMode( CACTI );
	if( _boostVMap.count( "daemon" ) != 0 ) operationMode( DAEMON );
	if( operationMode() == UNKNOWN ) throw std::invalid_argument( "Either one of the operation mode option \"full\" or \"cacti\" must be set" );
	
	
	// Create list of channels, skip duplicates. If no channels are given,
	// create list of all channels
	if( _boostVMap[ "channels" ].empty() ) {
		_channels.clear();
		_channels.reserve( LM50Device::countChannels );
		for( ChIdx i( 0 ); i < LM50Device::countChannels; i++ ) _channels.push_back(i);
	} else {
		_channels = _boostVMap[ "channels" ].as< ChList >();
		std::sort< ChList::iterator >( _channels.begin(), _channels.end() );
		ChList::iterator it( std::unique< ChList::iterator >( _channels.begin(), _channels.end() ) );
		_channels.resize( it - _channels.begin() );
		
		// Check if at least one channel remained, decrease each channel index
		// and ensure that the remaining channels indizes are in the correct range
		// Decreasing is necessary, because externally (to/from CLI) the indizes
		// are between 1 and 50, internally they are between 0 and 49
		std::ostringstream msg;
		msg << "The channels must be between 1 and " << LM50Device::countChannels;
		if( _channels.empty() ) throw std::domain_error( msg.str() );
		for( ChList::iterator it( _channels.begin() ); it != _channels.end(); it++ ) {
			if( --(*it) >= LM50Device::countChannels ) throw std::domain_error( msg.str() );
		}
	}
}

void ProgramOptions::operationMode( OperationMode m ) {
	assert( m != UNKNOWN );
	// Check that the operation mode has not been set before
	if( _operationMode != UNKNOWN ) throw std::invalid_argument( "The operation mode options \"full\", \"cacti\" and \"daemon\" are pairwise exclusive" );
	_operationMode = m;
}

}

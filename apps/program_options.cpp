#include "program_options.h"

#include <fstream>

namespace LM50 {

namespace po = boost::program_options;
using std::string;
using std::vector;
typedef vector< string > str_vector;


ProgramOptions::ProgramOptions() : \
	_commonOptions( "Common options" ),\
	_commonOptionsRRD( "Common options - \"RRD\" module" ),\
	_commonOptionsReport( "Common options - \"Report\" module" ),\
	_cmdLineOnlyOptions( "Command line only options" ),\
	_preOptionVMap(),\
	_commonOptionVMap(),\
	_rrdOptionVMap(),\
	_reportOptionVMap(),\
	_operationMode( UNKNOWN ),\
	_hasOptionHelp( false ),\
	_configFile(),\
	_host(),\
	_port(),\
	_foreground( false ),\
	_verbose( false ),\
	_channels(),
	_rrd( false ),
	_rrdFile(),
	_rrdPeriod( 30 ),
	_report( false ),
	_reportFile(),
	_reportPeriod( MONTHLY ),
	_reportRecipient() {
	
	_channels.reserve( LM50Device::countChannels );
	for( ChIdx i( 0 ); i < LM50Device::countChannels; i++ ) _channels.push_back(i+1);
	
	_cmdLineOnlyOptions.add_options()
		( "help", po::bool_switch( &_hasOptionHelp ), "Prints this help message and exits." )
		( "config,c", po::value< string >( &_configFile )->implicit_value( string( "/etc/lm50client.cnf" ) ), "Reads additional configuration options from file. If the same option is specified both on the command line and in the file, the command line option takes presedence." );
	
	_commonOptions.add_options()
		( "host,h", po::value< string >( &_host )->required(), "The DNS name of the LM50TCP+ to connect to." )
		( "port,p", po::value< string >( &_port )->default_value( string( "502") ), "The port on that the LM50TCP+ listens (default 502). The port can either be given as an integer or as a well-known servive name. E.g. \"http\" is identical to \"80\"." )
		( "mode,m", po::value< string >()->required(), "The operation mode of the program. Must be one out of the following values:\nhuman,h   \tWrites results to standard output in a nice readable layout.\ncacti,c   \tWrite results to standard output such that they can be parsed by cacti.\ndaemon,d  \tForks into background and polls the LM50TCP+ periodically." )
		( "workers,w", po::value< str_vector >()->multitoken()->default_value( str_vector(), "" ), "In daemon mode only: A list of workers (i.e. components) that are scheduled by the daemon and perform a specific task. Each worker has its own set of configuration options. The following workers are available:\nrrd     \tThis worker queries the device periodically and writes the results into a RRDTool file.\nreport  \tThis worker queries the device periodically and sends a report of the changes since the last query per email." )
		( "foreground,f", po::bool_switch( &_foreground ), "In daemon mode only: Do not fork into background but write operational log to standard output for debugging purpose" )
		( "verbose,v", po::bool_switch( &_verbose ), "In daemon mode only: Do not fork into background but write verbose log to standard output for debugging purpose" )
		( "channels,C", po::value< ChList >( &_channels )->multitoken()->default_value( _channels, "<all>" ), "Specifies the channels whose values are polled and processed. Multiple channel numbers must be seperated by white spaces. If the option is specified more than once, the lists of channels are joined. The channels are sorted increasingly and duplicates are skipped. E.g. \"-C 6 11 9 6 -C 11\" is equivalent to \"-C 6 9 11\". If the option is omitted, all available channels are polled." );
		
	_commonOptionsRRD.add_options()
		( "rrd.file", po::value< string >( &_rrdFile )->required(), "The path of the RRDTool file" )
		( "rrd.period", po::value< unsigned long >( &_rrdPeriod )->default_value( _rrdPeriod ), "Number of seconds between polling new values from device" );
		
	_commonOptionsReport.add_options()
		( "report.file", po::value< string >( &_reportFile )->required(), "The path to a (temporary) file in order to store the values between two consecutive reports" )
		( "report.period", po::value< string >()->default_value( "monthly" ), "The interval between two reports. Must be one out of the following values:\ndaily,d    \tCreates a report every midnight.\nweekly,w   \tCreates a report every midnight between sunday and monday.\nmonthly,m  \tCreates a report every midnight before the 1st of each month." )
		( "report.recipient", po::value< string >( &_reportRecipient )->required(), "The email address to send the report to" );
}


void ProgramOptions::print( std::ostream &s ) const {
	_commonOptions.print( s );
	s << std::endl;
	_commonOptionsRRD.print( s );
	s << std::endl;
	_commonOptionsReport.print( s );
	s << std::endl;
	_cmdLineOnlyOptions.print(s);
	s << std::endl;
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
	// The config file in case it is needed
	std::ifstream file;
	
	// Parse the command line with respect to command line only options first,
	// in order to check for the help option. If help option is set return
	// immediatly. We must call "allow_unregistered" to ignore options that are
	// not an element of _cmdLineOnlyOptions.
	po::store( po::command_line_parser( argCount, argVals ).options( _cmdLineOnlyOptions ).allow_unregistered().run(), _preOptionVMap );
	po::notify( _preOptionVMap );
	if( _hasOptionHelp ) return;
	
	// Parse the command line a second time but this time with respect to all
	// available command line options. If the first parsing showed that there
	// is a config file, parse this, too. Ignore unknown options again, because
	// there might be options for workers that are not enabled.
	po::store( po::command_line_parser( argCount, argVals ).options( _cmdLineOnlyOptions ).allow_unregistered().run(), _commonOptionVMap );
	po::store( po::command_line_parser( argCount, argVals ).options( _commonOptions ).allow_unregistered().run(), _commonOptionVMap );
	if( !_configFile.empty() ) {
		file.open( _configFile.c_str() );
		if( file.fail() ) throw std::invalid_argument( "Could not open config file" );
		po::store( po::parse_config_file( file, _commonOptions, true ), _commonOptionVMap );
	}
	po::notify( _commonOptionVMap );
	
	// Store operation mode
	string mode( _commonOptionVMap[ "mode" ].as< string >() );
	if( mode.compare( "human" ) == 0 || mode.compare( "h" ) == 0 ) operationMode( HUMAN );
	if( mode.compare( "cacti" ) == 0 || mode.compare( "c" ) == 0 ) operationMode( CACTI );
	if( mode.compare( "daemon" ) == 0 || mode.compare( "d" ) == 0 ) operationMode( DAEMON );
	if( operationMode() == UNKNOWN ) throw std::invalid_argument( "Either one of the operation modes \"human\", \"daemon\" or \"cacti\" must be set" );
	
	// Verbose must only be true, if foreground is set. Hence, disable "verbose"
	// in case "foreground" is not enabled.
	if( !_foreground ) _verbose = false;
	
	
	// Sort list of channels and remove duplicates
	std::sort< ChList::iterator >( _channels.begin(), _channels.end() );
	ChList::iterator it( std::unique< ChList::iterator >( _channels.begin(), _channels.end() ) );
	_channels.resize( it - _channels.begin() );
		
	// Ensure that the channels indizes are in the correct range
	// Decreasing is necessary, because externally (to/from CLI) the indizes
	// are between 1 and 50, internally they are between 0 and 49
	std::ostringstream msg;
	msg << "The channels must be between 1 and " << LM50Device::countChannels;
	for( ChList::iterator it( _channels.begin() ); it != _channels.end(); it++ ) {
		if( --(*it) >= LM50Device::countChannels ) throw std::domain_error( msg.str() );
	}
	
	// Obtain list of workers
	str_vector workers( _commonOptionVMap[ "workers" ].as< str_vector >() );
	while( ! workers.empty() ) {
		string w( workers.back() );
		workers.pop_back();
		if( w.compare( "rrd" ) == 0 ) _rrd = true;
		else if ( w.compare( "report" ) == 0 ) _report = true;
		else throw std::invalid_argument( string( "Unknown worker \"" ).append(w).append( "\"" ) );
	}
	
	// Parse options for worker "rrd"
	if( _rrd ) {
		po::store( po::command_line_parser( argCount, argVals ).options( _commonOptionsRRD ).allow_unregistered().run(), _rrdOptionVMap );
		if( file.is_open() ) {
			file.clear();
			file.seekg(0);
			if( file.fail() ) throw std::invalid_argument( "Could seek in config file" );
			po::store( po::parse_config_file( file, _commonOptionsRRD, true ), _rrdOptionVMap );
		}
		po::notify( _rrdOptionVMap );
	}
	
	// Parse options for worker "report"
	if( _report ) {
		po::store( po::command_line_parser( argCount, argVals ).options( _commonOptionsReport ).allow_unregistered().run(), _reportOptionVMap );
		if( file.is_open() ) {
			file.clear();
			file.seekg(0);
			if( file.fail() ) throw std::invalid_argument( "Could seek in config file" );
			po::store( po::parse_config_file( file, _commonOptionsReport, true ), _reportOptionVMap );
		}
		po::notify( _reportOptionVMap );
		
		string period( _reportOptionVMap[ "report.period" ].as< string >() );
		if( period.compare( "daily" ) == 0 || mode.compare( "d" ) == 0 ) _reportPeriod = DAILY;
		else if( period.compare( "weekly" ) == 0 || mode.compare( "w" ) == 0 ) _reportPeriod = WEEKLY;
		else if( period.compare( "monthly" ) == 0 || mode.compare( "m" ) == 0 ) _reportPeriod = MONTHLY;
		else throw std::invalid_argument( "The report period must be one out of \"daily\", \"weekly\" or \"monthly\"" );
	}
}

void ProgramOptions::operationMode( OperationMode m ) {
	assert( m != UNKNOWN );
	// Check that the operation mode has not been set before
	if( _operationMode != UNKNOWN ) throw std::invalid_argument( "The operation mode options \"human\", \"cacti\" and \"daemon\" are pairwise exclusive" );
	_operationMode = m;
}

}

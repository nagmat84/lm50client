#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <boost/program_options.hpp>
#include "lib/modbus.h"
#include <lib/mb_ascii.h>


static boost::program_options::variables_map cmdLineOptions;


static const uint16_t lm50addressVersion( 0x0578 );
static const uint16_t lm50addressSerial( 0x2710 );
static const uint16_t lm50addressMeter[50] = {
	0x0080, 0x0082, 0x0084, 0x0086, 0x0088, 0x008a, 0x008c, 0x008e,
	0x0090, 0x0092, 0x0094, 0x0096, 0x0098, 0x009a, 0x009c, 0x009e,
	0x00a0, 0x00a2, 0x00a4, 0x00a6, 0x00a8, 0x00aa, 0x00ac, 0x00ae,
	0x00b0, 0x00b2, 0x00b4, 0x00b6, 0x00b8, 0x00ba, 0x00bc, 0x00be,
	0x00c0, 0x00c2, 0x00c4, 0x00c6, 0x00c8, 0x00ca, 0x00cc, 0x00ce,
	0x00d0, 0x00d2, 0x00d4, 0x00d6, 0x00d8, 0x00da, 0x00dc, 0x00de,
	0x00e0, 0x00e2
};


/**
 * @throws std::exception& If any parser error occurs or if some mandatory
 * argument is missing
 * @return True, if "--help" was specified as an argument. In this case the
 * parseOptions( int, char*[] ) prints the help message. The caller is supposed
 * to termintate program execution or do whatever the caller thinks is
 * appropriate. In all other cases the return value is false.
 */
bool parseOptions( int argCount, char* argVals[] ) {
	boost::program_options::options_description podesc( "Allowed options" );
	
	try {
		podesc.add_options()
		( "help", "Prints this help message." )
		( "host,h", boost::program_options::value< std::string >(), "The DNS name of the LM50TCP+ to connect to." )
		( "port,p", boost::program_options::value< std::string >()->default_value( std::string( "502") ), "The port on that the LM50TCP+ listens (default 502). The port can either be given as an integer or as a well-known servive name. E.g. \"http\" is identical to \"80\"." )
		( "full,f", "Receives all available data from the LM50TCP+ and prints it in a nice human readable layout." )
		( "cacti,c", boost::program_options::value< std::vector<unsigned int> >()->multitoken(), "The argument are one or more numeric values between 1 and 50 seperated by white spaces. As an alternative the option can specified several times. Each number specifies a meter input whose value is received and printed in a format that is suitable for Cacti. The list of numbers is sorted increasingly and multiple equal numbers are only processed once. E.g. \"-cacti 6 11 9 6\" is processed as \"-cacti 6 9 11\"." );
	
		boost::program_options::store( boost::program_options::command_line_parser( argCount, argVals ).options( podesc ).run(), cmdLineOptions );
		boost::program_options::notify( cmdLineOptions );
		
		if( cmdLineOptions.count( "help" ) ) {
			std::cerr << podesc << std::endl;
			return true;
		}
		
		if( cmdLineOptions.count( "host" ) != 1 ) throw std::logic_error( "The host name to connect to must be specified" );
		if( cmdLineOptions.count( "port" ) != 1 ) throw std::logic_error( "The host name to connect to must be specified" );
		
		// Either "--full" must be specified or "--catci"
		if( cmdLineOptions.count( "full" ) > 0 && cmdLineOptions.count( "cacti" ) > 0 ) throw std::invalid_argument( "Options \"full\" and \"cacti\" cannot be specified both at the same time" );
		
		if( cmdLineOptions.count( "full" ) == 0 && cmdLineOptions.count( "cacti" ) == 0 ) throw std::invalid_argument( "Either option \"full\" or \"cacti\" must be specified" );
		
		// If cacti is specified, check that all numbers are between 1 and 50
		if( ! cmdLineOptions[ "cacti" ].empty() ) {
			const std::vector< unsigned int >& meterNos( cmdLineOptions[ "cacti" ].as< std::vector< unsigned int > >() );
			for( std::vector< unsigned int >::const_iterator it = meterNos.begin(); it != meterNos.end(); it++ ) {
				unsigned int meterNo( *it );
				if( 1 > meterNo || meterNo > 50 ) throw std::domain_error( "The meter input number must be between 1 and 50" );
			}
		}
		
		return false;
	} catch( ... ) {
		std::cerr << podesc << std::endl;
		throw;
	}
}


/**
 * Queries everything that might be useful from the device and prints the
 * received data to standard output in a nice human readable form.
 * Errors and their description are printed to standard output, too.
 * @return The error code. Equals zero if successfull.
 */
int modeFull() throw() {
	try {
		ModBus::TcpCommunication comm( cmdLineOptions[ "host" ].as< std::string >(), cmdLineOptions[ "port" ].as< std::string >() );
		
		std::cout << "===  LM-50TCP+ === " << std::endl << std::endl;
		
		// Read version information of the LM-50TCP+. This is a ASCII string with
		// at most 6 bytes (i.e. at most 5 letters and a trailing EOS).
		// The ASCII characters are located at the the 16-bit registers
		// 0x0578, 0x0579 and 0x057a
		ModBus::Datagram::ReadHoldingRegistersReq reqVersion( 1, 1, lm50addressVersion, 3 );
		ModBus::TcpRequestAndReply rarVersion( comm.createRequestAndReply( reqVersion ) );
		
		// Read serial number of the LM-50TCP+. This is a 32-bit unsigned integer
		// and is located at the 16-bit registers 0x2710 and 0x2711.
		ModBus::Datagram::ReadHoldingRegistersReq reqSerial( 1, 1, lm50addressSerial, 2 );
		ModBus::TcpRequestAndReply rarSerial( comm.createRequestAndReply( reqSerial ) );
		
		// Try to obtain replies and print them
		std::cout << "Version :  ";
		rarVersion.run();
		const ModBus::Datagram::Base* resBase = rarVersion.response();
		const ModBus::Datagram::ErrorRes* resError = dynamic_cast< const ModBus::Datagram::ErrorRes* >( resBase );
		const ModBus::Datagram::ReadHoldingRegistersRes* resRegister = dynamic_cast< const ModBus::Datagram::ReadHoldingRegistersRes* >( resBase );
		if( resBase == nullptr ) {
			std::cout << "Time out" << std::endl;
		} else {
			if( resRegister ) {
				ModBus::Interpreter::ASCII< ModBus::Datagram::ReadHoldingRegistersRes > version( *resRegister );
				std::cout << version.string() << std::endl;
			} else if( resError ) {
				std::cout << "ModBus error  -  " << resError->exceptionMessage() << std::endl;
			} else {
				std::cout << "Unexpected return value" << std::endl;
			}
		}
		
		std::cout << "Serial  :  ";
		rarSerial.run();
		resBase = rarSerial.response();
		resError = dynamic_cast< const ModBus::Datagram::ErrorRes* >( resBase );
		resRegister = dynamic_cast< const ModBus::Datagram::ReadHoldingRegistersRes* >( resBase );
		if( resBase == nullptr ) {
			std::cout << "Time out" << std::endl;
		} else {
			if( resRegister ) {
				ModBus::Interpreter::UInt32< ModBus::Datagram::ReadHoldingRegistersRes > serial( *resRegister );
				std::cout << serial.value( 0 ) << std::endl;
			} else if( resError ) {
				std::cout << "ModBus error  -  " << resError->exceptionMessage() << std::endl;
			} else {
				std::cout << "Unexpected return value" << std::endl;
			}
		}
		
		
		// Now read all meter values
		// The first value (32-bit) is located at 0x0080 and 0x0081
		// The 50th value (32-bit)  is located at 0x00e2 and 0x00e3
		// This means starting with adress 0x0080 there are 100 1-bit registers to
		// read
		ModBus::Datagram::ReadInputRegistersReq reqMeters( 1, 1, lm50addressMeter[0], 100 );
		ModBus::TcpRequestAndReply rarMeters( comm.createRequestAndReply( reqMeters ) );
		
		std::cout << "Reading meter inputs ... " << std::flush;
		rarMeters.run();
		resBase = rarMeters.response();
		resError = dynamic_cast< const ModBus::Datagram::ErrorRes* >( resBase );
		const ModBus::Datagram::ReadInputRegistersRes* resIRegister = dynamic_cast< const ModBus::Datagram::ReadInputRegistersRes* >( resBase );
		
		if( resBase == nullptr ) {
			std::cout << "Time out" << std::endl;
		} else {
			if( resIRegister ) {
				std::cout << "done" << std::endl;
				ModBus::Interpreter::UInt32< ModBus::Datagram::ReadInputRegistersRes > meters( *resIRegister );
				for( size_t j = 0; j < meters.size(); j++ ) {
					std::cout << "Meter " << std::setw( 2 ) << (j+1) << ":   " << std::setw(12) << meters.value( j ) << std::endl;
				}
			} else if( resError ) {
				std::cout << "ModBus error  -  " << resError->exceptionMessage() << std::endl;
			} else {
				std::cout << "Unexpected return value" << std::endl;
			}
		}
		
		
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


/**
 * Queries those input registers that are specified in the command line options
 * and prints their values to standard output such that they can be used by
 * Cacti. If an error occurs the erroneous value is skipped silently and the
 * function tries to continue in the best ways possible.
 * No error message is printed to standard output, because this message cannot
 * be fed into Cacti anyway. But the return value is set to the error code of
 * the last error that occured.
 * @return The latest error code. Equals zero if successfull.
 */
int modeCacti() throw() {
	// Save old locale and set locale to C locale
	std::ostringstream cactiOutput;
	cactiOutput.imbue( std::locale( "C" ) );
	cactiOutput.fill( '0' );
	
	
	int lastError( 0 );
	
	try {
		// Sort the input registers and removes duplicates
		std::vector< unsigned int > metersNos( cmdLineOptions[ "cacti" ].as< std::vector< unsigned int > >() );
		std::sort< std::vector< unsigned int >::iterator >( metersNos.begin(), metersNos.end() );
		std::vector< unsigned int >::iterator it( std::unique< std::vector< unsigned int >::iterator >( metersNos.begin(), metersNos.end() ) );
		metersNos.resize( it - metersNos.begin() );
		assert( !metersNos.empty() );
		
		ModBus::TcpCommunication comm( cmdLineOptions[ "host" ].as< std::string >(), cmdLineOptions[ "port" ].as< std::string >() );
		
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
		
		if( metersNos.size() == 1 ) {
			unsigned int meterNo( metersNos.front() );
			
			assert( 1 <= meterNo && meterNo <= 50 );
			ModBus::Datagram::ReadInputRegistersReq reqMeter( 1, 1, lm50addressMeter[ meterNo-1 ], 2 );
			ModBus::TcpRequestAndReply rarMeter( comm.createRequestAndReply( reqMeter ) );
			
			rarMeter.run();
			const ModBus::Datagram::Base* resBase = rarMeter.response();
			const ModBus::Datagram::ErrorRes* resError = dynamic_cast< const ModBus::Datagram::ErrorRes* >( resBase );
			const ModBus::Datagram::ReadInputRegistersRes* resIRegister = dynamic_cast< const ModBus::Datagram::ReadInputRegistersRes* >( resBase );
			
			if( resBase == nullptr ) {
				// timeout
				lastError = -1;
			} else {
				if( resIRegister ) {
					ModBus::Interpreter::UInt32< ModBus::Datagram::ReadInputRegistersRes > meterVal( *resIRegister );
					assert( meterVal.size() == 1 );
					cactiOutput << meterVal.value( 0 );
				} else if( resError ) {
					// modbus error
					lastError = resError->exceptionCode();
				} else {
					// unexpected error
					lastError = -1;
				}
			}
			std::cout << cactiOutput.str() << std::flush;
		} else {
			unsigned int meterNo = 0;

			try {
				it = metersNos.begin();
				meterNo = *it;
				
				assert( 1 <= meterNo && meterNo <= 50 );
				ModBus::Datagram::ReadInputRegistersReq reqMeter( 1, 1, lm50addressMeter[ meterNo-1 ], 2 );
				ModBus::TcpRequestAndReply rarMeter( comm.createRequestAndReply( reqMeter ) );
				
				rarMeter.run();
				const ModBus::Datagram::Base* resBase = rarMeter.response();
				const ModBus::Datagram::ErrorRes* resError = dynamic_cast< const ModBus::Datagram::ErrorRes* >( resBase );
				const ModBus::Datagram::ReadInputRegistersRes* resIRegister = dynamic_cast< const ModBus::Datagram::ReadInputRegistersRes* >( resBase );
				
				if( resBase == nullptr ) {
					// timeout
					cactiOutput << "meter" << std::setw( 2 ) << meterNo << ":nan";
					lastError = -1;
				} else {
					if( resIRegister ) {
						ModBus::Interpreter::UInt32< ModBus::Datagram::ReadInputRegistersRes > meterVal( *resIRegister );
						assert( meterVal.size() == 1 );
						cactiOutput << "meter" << std::setw( 2 ) << meterNo << ':' << meterVal.value( 0 );
					} else if( resError ) {
						// modbus error
						cactiOutput << "meter" << std::setw( 2 ) << meterNo << ":nan";
						lastError = resError->exceptionCode();
					} else {
						// unexpected error
						cactiOutput << "meter" << std::setw( 2 ) << meterNo << ":nan";
						lastError = -1;
					}
				}
			} catch( boost::system::error_code& e ) {
				cactiOutput << "meter" << std::setw( 2 ) << meterNo << ":nan";
				lastError = e.value();
			} catch( ... ) {
				cactiOutput << "meter" << std::setw( 2 ) << meterNo << ":nan";
				lastError = -1;
			}
			
			for( ++it; it != metersNos.end(); it++ ) {
				try {
					meterNo = *it;
					assert( 1 <= meterNo && meterNo <= 50 );
					ModBus::Datagram::ReadInputRegistersReq reqMeter( 1, 1, lm50addressMeter[ meterNo-1 ], 2 );
					ModBus::TcpRequestAndReply rarMeter( comm.createRequestAndReply( reqMeter ) );
					
					rarMeter.run();
					const ModBus::Datagram::Base* resBase = rarMeter.response();
					const ModBus::Datagram::ErrorRes* resError = dynamic_cast< const ModBus::Datagram::ErrorRes* >( resBase );
					const ModBus::Datagram::ReadInputRegistersRes* resIRegister = dynamic_cast< const ModBus::Datagram::ReadInputRegistersRes* >( resBase );
					
					if( resBase == nullptr ) {
						// timeout
						cactiOutput << " meter" << std::setw( 2 ) << meterNo << ":nan";
						lastError = -1;
					} else {
						if( resIRegister ) {
							ModBus::Interpreter::UInt32< ModBus::Datagram::ReadInputRegistersRes > meterVal( *resIRegister );
							assert( meterVal.size() == 1 );
							cactiOutput<< " meter" << std::setw( 2 ) << meterNo << ':' << meterVal.value( 0 );
						} else if( resError ) {
							// modbus error
							cactiOutput << " meter" << std::setw( 2 ) << meterNo << ":nan";
							lastError = resError->exceptionCode();
						} else {
							// unexpected error
							cactiOutput << " meter" << std::setw( 2 ) << meterNo << ":nan";
							lastError = -1;
						}
					}
				} catch( boost::system::error_code& e ) {
					cactiOutput << " meter" << std::setw( 2 ) << meterNo << ":nan";
					lastError = e.value();
				} catch( ... ) {
					cactiOutput << " meter" << std::setw( 2 ) << meterNo << ":nan";
					lastError = -1;
				}
			}
			std::cout << cactiOutput.str() << std::flush;
		}
	} catch ( boost::system::error_code& e ) {
		return e.value();
	} catch ( ... ) {
		return -1;
	}
	
	return lastError;
}



int main( int argCount, char* argVals[] ) {
	try {
		std::locale loc( "" );
		std::locale::global( loc );
		std::cout.imbue( loc );
		std::cerr.imbue( loc );
		std::clog.imbue( loc );
		std::cin.imbue( loc );
		if( parseOptions( argCount, argVals ) ) return 0;
	} catch( std::exception& e ) {
		std::cerr << "Error:  " << e.what() << std::endl;
		return -1;
	}
	catch( ... ) {
		std::cerr << "Unknown fatal error" << std::endl;
		return -1;
	}
	
	if( cmdLineOptions.count( "full" ) > 0 ) {
		return modeFull();
	} else {
		return modeCacti();
	}
	
	return 0;
}

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include "lib/modbus.h"
#include "lib/mb_ascii.h"
#include "core/program_options.h"
#include "core/lm50device.h"

using namespace LM50;

static ProgramOptions cmdLineOptions;


/**
 * Queries everything that might be useful from the device and prints the
 * received data to standard output in a nice human readable form.
 * Errors and their description are printed to standard output, too.
 * @return The error code. Equals zero if successfull.
 */
int modeFull() throw() {
	try {
		ModBus::TcpCommunication comm( cmdLineOptions.host(), cmdLineOptions.port() );
		
		std::cout << "===  LM-50TCP+ === " << std::endl << std::endl;
		
		// Read version information of the LM-50TCP+. This is a ASCII string with
		// at most 6 bytes (i.e. at most 5 letters and a trailing EOS).
		// The ASCII characters are located at the the 16-bit registers
		// 0x0578, 0x0579 and 0x057a
		ModBus::Datagram::ReadHoldingRegistersReq reqVersion( 1, 1, LM50Device::hwAddrVersion, 3 );
		ModBus::TcpRequestAndReply rarVersion( comm.createRequestAndReply( reqVersion ) );
		
		// Read serial number of the LM-50TCP+. This is a 32-bit unsigned integer
		// and is located at the 16-bit registers 0x2710 and 0x2711.
		ModBus::Datagram::ReadHoldingRegistersReq reqSerial( 1, 1, LM50Device::hwAddrSerial, 2 );
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
		ModBus::Datagram::ReadInputRegistersReq reqMeters( 1, 1, LM50Device::hwAddrChannel(1), 100 );
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
		ModBus::TcpCommunication comm( cmdLineOptions.host(), cmdLineOptions.port() );
		
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
		
		if( cmdLineOptions.channels().size() == 1 ) {
			unsigned int meterNo( cmdLineOptions.channels().front() );
			
			assert( 1 <= meterNo && meterNo <= 50 );
			ModBus::Datagram::ReadInputRegistersReq reqMeter( 1, 1, LM50Device::hwAddrChannel( meterNo ), 2 );
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
			
			ProgramOptions::ChList::const_iterator it;
			try {
				it = cmdLineOptions.channels().begin();
				meterNo = *it;
				
				assert( 1 <= meterNo && meterNo <= 50 );
				ModBus::Datagram::ReadInputRegistersReq reqMeter( 1, 1, LM50Device::hwAddrChannel( meterNo ), 2 );
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
			
			for( ++it; it != cmdLineOptions.channels().end(); it++ ) {
				try {
					meterNo = *it;
					assert( 1 <= meterNo && meterNo <= 50 );
					ModBus::Datagram::ReadInputRegistersReq reqMeter( 1, 1, LM50Device::hwAddrChannel( meterNo ), 2 );
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
		cmdLineOptions.parse( argCount, argVals );
	} catch( std::exception& e ) {
		std::cerr << "Error:  " << e.what() << std::endl;
		return -1;
	}
	catch( ... ) {
		std::cerr << "Unknown fatal error" << std::endl;
		return -1;
	}
	
	if ( cmdLineOptions.hasOptionHelp() ) {
		cmdLineOptions.print( std::cout );
		return 0;
	}
	
	switch( cmdLineOptions.operationMode() ) {
		case ProgramOptions::FULL:
			return modeFull();
		case ProgramOptions::CACTI:
			return modeCacti();
		default:
			std::cerr << "Unknown fatal error" << std::endl;
			return -1;
	}
	
	return 0;
}

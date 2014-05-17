#include "lib/mb_errorres.h"

#include <stdexcept>
#include <cstring>

namespace ModBus {
namespace Datagram {


ErrorRes::ErrorRes( const char* rawDatagram, unsigned int length ) : Base() {
	// An error reply datagram is exactly 9 bytes long. The raw data might
	// be longer if some additional bogus has been captured at the end
	if( length < 9 ) throw std::length_error( "rawDatagram must be at least 9 bytes long" );
	
	const Header* header = reinterpret_cast<const Header*>( rawDatagram );
	if( header->protocolID != 0x00 ) throw std::invalid_argument( "rawDatagram does not seem to start with a ModBus header" );
	if( ntohs( header->length ) != 3 ) throw std::invalid_argument( "rawDatagram does not have the correct length for an ModBus error datagram" );
	
	dgram = new ErrorResDatagram;
	memcpy( dgram->rawData, rawDatagram, 9 );
}


ErrorRes::ErrorRes( const ErrorRes& other ) : Base( other ) {
	dgram = new ErrorResDatagram;
	memcpy( dgram->rawData, other.dgram->rawData, 9 );
}


ErrorRes::~ErrorRes() {
	if( dgram ) delete dgram;
}

std::string ErrorRes::exceptionMessage() const {
	switch( exceptionCode() ) {
		case Exception::IllegalFunction:
			return std::string( "Illegal function" );
		case Exception::IllegalAddress:
			return std::string( "Illegal data address" );
		case Exception::IllegalValue:
			return std::string( "Illegal data value" );
		case Exception::DeviceFailure:
			return std::string( "Slave device failure" );
		case Exception::Acknowledge:
			return std::string( "Acknowledge" );
		case Exception::DeviceBusy:
			return std::string( "Slave device busy" );
		case Exception::GatewayUnavailable:
			return std::string( "Gateway unavailable" );
		case Exception::ParityError:
			return std::string( "Parity error" );
		case Exception::TargetFailure:
			return std::string( "Target failure" );
		default:
			return std::string( "Unknown exception" );
	}
}



}  // namespace Datagram
}  // namespace ModBus

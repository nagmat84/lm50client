#include "lib/mb_base.h"
#include "lib/mb_errorres.h"
#include "lib/mb_generic.h"
#include "lib/mb_rhregreq.h"
#include "lib/mb_rhregres.h"
#include "lib/mb_riregreq.h"
#include "lib/mb_riregres.h"

#include <stdexcept>

namespace ModBus {
namespace Datagram {


Base* Base::createObject( const char* rawDatagram, size_t length ) {
	// Check if rawDatagram is complete. This also performs some sanity checks
	// like correct protocolID and so on
	if( missingBytes( rawDatagram, length ) > 0 ) throw std::length_error( "rawDatagram is truncated" );
	
	const Header* header = reinterpret_cast<const Header*>( rawDatagram );
	// If the error bit is set, return special error object
	if( header->funcCode & Function::Error ) return new ErrorRes( rawDatagram, length );
	
	unsigned int l = ntohs( header->length );
	
	switch( header->funcCode ) {
		case Function::ReadHoldingRegisters:
			// The length field of a request is always 6 (1 byte unit id, 1 byte
			// function code, 2 bytes start address, 2 bytes quantity)
			// The length field of a response is always odd (1 byte unit id, 1 byte
			// function code, 1 byte counter and an even number of bytes holding the
			// value)
			if( l == 6 ) return new ReadHoldingRegistersReq( rawDatagram, length );
			return new ReadHoldingRegistersRes( rawDatagram, length );
		case Function::ReadInputRegisters:
			// See comment in case Function::ReadHoldingRegisters:
			if( l == 6 ) return new ReadInputRegistersReq( rawDatagram, length );
			return new ReadInputRegistersRes( rawDatagram, length );
			break;
		default:
			return new Generic( rawDatagram, length );
			break;
	}
	assert( false );
}


size_t Base::missingBytes( const char* rawDatagram, size_t length ) {
	if( length < 6 ) throw std::length_error( "rawDatagram must be at least 6 bytes long" );
	const Header* header = reinterpret_cast<const Header*>( rawDatagram );
	if( header->protocolID != 0x00 ) throw std::invalid_argument( "rawDatagram does not seem to start with a ModBus header" );
	unsigned int mbLength = ntohs( header->length );
	if( length >= mbLength + 6 ) return 0;
	return ( mbLength + 6 - length );
}


}  // namespace Datagram
}  // namespace ModBus

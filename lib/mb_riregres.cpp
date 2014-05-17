#include "lib/mb_riregres.h"

#include <cstring>

namespace ModBus {
namespace Datagram {


ReadInputRegistersRes::ReadInputRegistersRes( const char* rawDatagram, unsigned int length ): Base() {
	// An read input registers response datagram is at least 9 bytes long.
	// (8 bytes header and 1 byte counter)
	// The raw data might be longer if some additional bogus has been captured
	// at the end
	if( length < 9 ) throw std::length_error( "rawDatagram must be at least 9 bytes long" );
	
	const Header* header = reinterpret_cast<const Header*>( rawDatagram );
	if( header->protocolID != 0x00 ) throw std::invalid_argument( "rawDatagram does not seem to start with a ModBus header" );
	if( header->funcCode != Function::ReadInputRegisters ) throw std::invalid_argument( "rawDatagram does not have the correct function code" );
	
	unsigned int mbLength = ntohs( header->length );
	if( length < mbLength + 6 ) throw std::length_error( "rawDatagram is truncated" );
	
	dgram = reinterpret_cast< RIRegResDatagram* >( new char[mbLength+6] );
	memcpy( dgram->rawData, rawDatagram, mbLength+6 );
	
	// In an read holding register reply the length is stored twice. Firstly it
	// is stored in the length field of the header as required for every ModBus
	// datagram. This length field is the datagram's length except for the first
	// six bytes (transaction id, protocol id, length field)
	// Secondly, the length of the returned values is stored ("byte count" in
	// ModBus terminology.) This is the length without the 8 byte header and
	// the byte count itsself.
	// Hence, difference between length and byte count is fixed to 3. Otherwise
	// the datagram is corrupted
	if( mbLength - dgram->structData.byteCount != 3 ) {
		delete[] reinterpret_cast< char* >( dgram );
		throw std::invalid_argument( "rawDatagram seems to be corrupted" );
	}
}


ReadInputRegistersRes::ReadInputRegistersRes( const ReadInputRegistersRes& other ): Base( other ) {
	const unsigned int realLength = other.totalLength();
	dgram = reinterpret_cast< RIRegResDatagram* >( new char[ realLength ] );
	memcpy( dgram->rawData, other.dgram->rawData, realLength );
}


ReadInputRegistersRes::~ReadInputRegistersRes() {
	if( dgram ) delete[] reinterpret_cast< char* >( dgram );
}


}  // namespace Datagram
}  // namespace ModBus

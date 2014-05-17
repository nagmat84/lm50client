#include "lib/mb_generic.h"

#include <stdexcept>
#include <cstring>

namespace ModBus {
namespace Datagram {

Generic::Generic( const char* rawDatagram, unsigned int length ) : Base() {
	// Check if rawDatagram is complete. This also performs some sanity checks
	// like correct protocolID and so on
	if( missingBytes( rawDatagram, length ) > 0 ) throw std::length_error( "rawDatagram is truncated" );
	
	const Header* header = reinterpret_cast<const Header*>( rawDatagram );
	const unsigned int realLength = ntohs( header->length ) + 6;
	dgram = reinterpret_cast< GenericDatagram* >( new char[ realLength ] );
	memcpy( dgram->rawData, rawDatagram, realLength );
	
}


Generic::Generic( const Generic& other ) : Base( other ) {
	const unsigned int realLength = other.totalLength();
	dgram = reinterpret_cast< GenericDatagram* >( new char[ realLength ] );
	memcpy( dgram->rawData, other.dgram->rawData, realLength );
}


Generic::~Generic() {
	if( dgram != nullptr ) {
		delete[] reinterpret_cast< char* >( dgram );
	}
}


}  // namespace Datagram
}  // namespace ModBus

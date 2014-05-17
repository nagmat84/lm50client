#include "lib/mb_riregreq.h"

#include <cstring>

namespace ModBus {
namespace Datagram {

ReadInputRegistersReq::ReadInputRegistersReq( TransactionID transaction, UnitID unit, u_int16_t address, u_int16_t quantity ) {
	if( quantity == 0 || quantity > 0x7d ) throw std::out_of_range( "Number of registers to read must be between 1 and 0x7d" );
	
	dgram = new RIRegReqDatagram;
	dgram->structData.header.transactionID = htons( transaction );
	dgram->structData.header.protocolID = 0x0000;
	dgram->structData.header.length = htons( 6 );
	dgram->structData.header.unitID = unit;
	dgram->structData.header.funcCode = Function::ReadInputRegisters;
	dgram->structData.address = htons( address );
	dgram->structData.quantity = htons( quantity );
}


ReadInputRegistersReq::ReadInputRegistersReq( const char* rawDatagram, unsigned int length ): Base() {
	// An read holding registers request datagram is exactly 12 bytes long.
	// The raw data might/ be longer if some additional bogus has been captured
	// at the end
	if( length < 12 ) throw std::length_error( "rawDatagram must be at least 12 bytes long" );
	
	const Header* header = reinterpret_cast<const Header*>( rawDatagram );
	if( header->protocolID != 0x00 ) throw std::invalid_argument( "rawDatagram does not seem to start with a ModBus header" );
	if( header->funcCode != Function::ReadInputRegisters ) throw std::invalid_argument( "rawDatagram does not have the correct function code" );
	if( ntohs( header->length ) != 6 ) throw std::invalid_argument( "rawDatagram does not have the correct length" );
	
	dgram = new RIRegReqDatagram;
	memcpy( dgram->rawData, rawDatagram, 12 );
}


ReadInputRegistersReq::ReadInputRegistersReq( const ReadInputRegistersReq& other ): Base( other ) {
	dgram = new RIRegReqDatagram;
	memcpy( dgram->rawData, other.dgram->rawData, 12 );
}


ReadInputRegistersReq::~ReadInputRegistersReq() {
	if( dgram ) delete dgram;
}


}  // namespace Datagram
}  // namespace ModBus

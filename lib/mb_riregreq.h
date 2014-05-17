#ifndef _MB_RIREGREQ_H_
#define _MB_RIREGREQ_H_

//include <sys/types.h>
#include <netinet/in.h>
#include <cassert>
#include <stdexcept>

#include "lib/mb_base.h"
#include "lib/nullptr.h"

namespace ModBus {
namespace Datagram {


class ReadInputRegistersReq : public Base {
	protected:
		// The low level representation of a ModBus telegram. The structure is
		// never exported to the outside, hence it can be protected.
		struct RIRegReqDatagram {
			union {
				char rawData[12];
				struct {
					Header header;
					u_int16_t address;
					u_int16_t quantity;
				} structData;
			};
		};
		
	public:
		ReadInputRegistersReq( TransactionID transaction, UnitID unit, u_int16_t address, u_int16_t quantity );
		
		/**
		* @param rawDatagram The raw bytes
		* @param length The length of the char array. I.e. it is NOT the length
		* as given by length field of the ModBus telegram, but it is the length
		* of the uninterpreted byte stream
		*/
		ReadInputRegistersReq( const char* rawDatagram, unsigned int length );
		
		ReadInputRegistersReq( const ReadInputRegistersReq& ReadInputRegisters );
		
		virtual ~ReadInputRegistersReq();
		
	public:
		
		virtual Base* copy() const {
			return new ReadInputRegistersReq( *this );
		}
		
		virtual size_t length() const {
			assert( dgram != nullptr );
			assert( ntohs( dgram->structData.header.length ) == 6 );
			return 6;
		}
		
		/**
		 * @return The total number of bytes that are required to store or transmit
		 * the datagram. I.e. the length of the ModBus datagram with the ModBus
		 * header included. This is supposed to equal Base::length() + 6.
		 */
		virtual size_t totalLength() const {
			assert( dgram != nullptr );
			assert( ntohs( dgram->structData.header.length ) == 6 );
			return 12;
		}
		
		virtual FuncCode funcCode() const {
			assert( dgram != nullptr );
			assert( dgram->structData.header.funcCode == Function::ReadInputRegisters );
			return Function::ReadInputRegisters;
		}
		
		virtual TransactionID transactionID() const {
			assert( dgram != nullptr );
			return ntohs( dgram->structData.header.transactionID );
		}
		
		virtual void transactionID( TransactionID id ) {
			assert( dgram != nullptr );
			dgram->structData.header.transactionID = htons( id );
		}
		
		virtual UnitID unitID() const {
			assert( dgram != nullptr );
			return dgram->structData.header.unitID;
		}
		
		virtual void unitID( UnitID id ) {
			assert( dgram != nullptr );
			dgram->structData.header.unitID = htons( id );
		}
		
		u_int16_t address() const {
			assert( dgram != nullptr );
			return ntohs( dgram->structData.address );
		}
		
		void address( u_int16_t a ) {
			assert( dgram != nullptr );
			dgram->structData.address = htons( a );
		}
		
		u_int16_t quantity() const {
			assert( dgram != nullptr );
			return ntohs( dgram->structData.quantity );
		}
		
		void quantity( u_int16_t n ) {
			assert( dgram != nullptr );
			if( n == 0 || n > 0x7d ) throw std::out_of_range( "Number of registers to read must be between 1 and 0x7d" );
			dgram->structData.quantity = htons( n );
		}
		
		virtual const char* rawDatagram() const {
			assert( dgram != nullptr );
			return dgram->rawData;
		}
		
	protected:
		RIRegReqDatagram* dgram;
};

} // namespace Datagram
} // namespace ModBus

#endif

#ifndef _MB_RIREGRES_H_
#define _MB_RIREGRES_H_

#include <netinet/in.h>
#include <cassert>
#include <stdexcept>

#include "lib/mb_base.h"
#include "lib/nullptr.h"

namespace ModBus {
namespace Datagram {

class ReadInputRegistersRes : public Base {
	protected:
		// The low level representation of a ModBus telegram. The structure is
		// never exported to the outside, hence it can be protected.
		struct RIRegResDatagram {
			union {
				char rawData[1];  // length of this array is length of value + 9
				struct {
					Header header;
					uint8_t byteCount;  // The length of value, always an even number
					char value[1];      // The values of the read registers. As the registers are 16bit, the array always has an even ĺength
				} structData;
			};
		};
		
	public:
		/**
		* @param rawDatagram The raw bytes
		* @param length The length of the char array. I.e. it is NOT the length
		* as given by length field of the ModBus telegram, but it is the length
		* of the uninterpreted byte stream
		*/
		ReadInputRegistersRes( const char* rawDatagram, unsigned int length );
		
		ReadInputRegistersRes( const ReadInputRegistersRes& ReadInputRegisters );
		
		virtual ~ReadInputRegistersRes();
		
	public:
		
		virtual Base* copy() const {
			return new ReadInputRegistersRes( *this );
		}
		
		virtual size_t length() const {
			assert( dgram != nullptr );
			return ntohs( dgram->structData.header.length );
		}
		
		/**
		 * @return The total number of bytes that are required to store or transmit
		 * the datagram. I.e. the length of the ModBus datagram with the ModBus
		 * header included. This is supposed to equal
		 * ReadInputRegistersRes::length() + 6.
		 */
		virtual size_t totalLength() const {
			assert( dgram != nullptr );
			return length() + 6;
		}
		
		/**
		 * @return The length of the value returned. As ModBus is 16bit oriented
		 * this value is always even. It must equal
		 * ReadHoldingRegistersRes::length() - 3.
		 */
		size_t byteCount() const {
			assert( dgram != nullptr );
			return dgram->structData.byteCount;
		}
		
		char value( size_t index ) const {
			assert( dgram != nullptr );
			if( index >= byteCount() ) throw std::out_of_range( "given index is greater than number of bytes" );
			return dgram->structData.value[ index ];
		}
		
		const char* values() const {
			assert( dgram != nullptr );
			return dgram->structData.value;
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
		
		virtual const char* rawDatagram() const {
			assert( dgram != nullptr );
			return dgram->rawData;
		}
		
	protected:
		RIRegResDatagram* dgram;
};

} // namespace Datagram
} // namespace ModBus

#endif

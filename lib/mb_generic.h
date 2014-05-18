#ifndef _MB_GENERIC_H_
#define _MB_GENERIC_H_

#include <netinet/in.h>
#include <cassert>

#include "lib/mb_base.h"
#include "lib/nullptr.h"

namespace ModBus {
namespace Datagram {


/**
 * There exists one additional class ModBus::Datagram::Generic that represents
 * a "raw" ModBus telegram, i.e. a ModBus header followed by unstructered bytes
 * of data. This generic class is a place-holder for all ModBus datagrams as
 * long as not all specialized classes exists.
 */
class Generic : public Base {
	protected:
		// The low level representation of a ModBus telegram. The structure is
		// never exported to the outside, hence it can be protected.
		struct GenericDatagram {
			union {
				char rawData[1];  // length of this array is length of funcData + 8 
				struct {
					Header header;
					char funcData[1];
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
		Generic( const char* rawDatagram, unsigned int length );
		
    Generic( const Generic& other );
		
		virtual ~Generic();
		
	public:
		
		virtual Generic* copy() const {
			return new Generic( *this );
		}
		
		virtual size_t length() const {
			assert( dgram != nullptr );
			return ntohs( dgram->structData.header.length );
		}
		
		/**
		 * @return The total number of bytes that are required to store or transmit
		 * the datagram. I.e. the length of the ModBus datagram with the ModBus
		 * header included. This is supposed to equal Base::length() + 6.
		 */
		virtual size_t totalLength() const {
			assert( dgram != nullptr );
			return length() + 6;
		}
		
		virtual FuncCode funcCode() const {
			assert( dgram != nullptr );
			return dgram->structData.header.funcCode;
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
		GenericDatagram* dgram;
};

} // namespace Datagram
} // namespace ModBus

#endif

#ifndef _MB_ERRORRES_H_
#define _MB_ERRORRES_H_

//include <sys/types.h>
#include <netinet/in.h>
#include <cassert>
#include <string>

#include "lib/mb_base.h"
#include "lib/nullptr.h"

namespace ModBus {
namespace Datagram {


class ErrorRes : public Base {
	protected:
		// The low level representation of a ModBus Exception telegram. The
		// function data is only one byte long an stores the exception code.
		// Hence the rawData is always 9 bytes and the ModBus length field
		// always equals 3 (1 byte for unit id, function code and exception code)
		struct ErrorResDatagram {
			union {
				char rawData[9];
				struct {
					Header header;
					ExceptionCode exCode;
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
		ErrorRes( const char* rawDatagram, unsigned int length );
		
		ErrorRes( const ErrorRes& other );
		
		virtual ~ErrorRes();
		
	public:
		
		virtual Base* copy() const {
			return new ErrorRes( *this );
		}
		
		virtual size_t length() const {
			assert( dgram != nullptr );
			assert( ntohs( dgram->structData.header.length ) == 3 );
			return 3;
		}
		
		/**
		 * @return The total number of bytes that are required to store or transmit
		 * the datagram. I.e. the length of the ModBus datagram with the ModBus
		 * header included. This is supposed to equal Base::length() + 6.
		 */
		virtual size_t totalLength() const {
			assert( dgram != nullptr );
			assert( ntohs( dgram->structData.header.length ) == 3 );
			return 9;
		}
		
		/**
		 * @return The function code without the error bit. (As this class
		 * represents an error, the error bit must be set anyway.) The error bit
		 * is masked out by purpose, so that the resulting function code can
		 * easily be compared against the remaining function codes
		 */
		virtual FuncCode funcCode() const {
			assert( dgram != nullptr );
			assert( dgram->structData.header.funcCode & Function::Error );
			return ( dgram->structData.header.funcCode & ~Function::Error );
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
		
		ExceptionCode exceptionCode() const {
			assert( dgram != nullptr );
			assert( dgram->structData.header.funcCode & Function::Error );
			return dgram->structData.exCode;
		}
		
		std::string exceptionMessage() const;
		
		virtual const char* rawDatagram() const {
			assert( dgram != nullptr );
			return dgram->rawData;
		}
		
	protected:
		ErrorResDatagram* dgram;
};

} // namespace Datagram
} // namespace ModBus

#endif

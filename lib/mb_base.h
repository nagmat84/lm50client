#ifndef _MB_BASE_H_
#define _MB_BASE_H_

#include "lib/mb_constants.h"

namespace ModBus {
namespace Datagram {


/**
 * The abstract base class for all clases that represents a ModBus datagram.
 * This base class does not hold any data itsself but only defines some
 * functions that must be re-implemented by child classes. In the final version
 * of this library there exists one specialized class for each type of
 * ModBus datagram, i.e. for each function code (see ModBus::Datagram::Function)
 *
 * This class contains static factory methods (class methods) to construct
 * objects of specialized classes depending on the raw datagram bytes
 */
class Base {
	protected:
		Base() {};
		Base( const Base& ) {};
		
	public:
		virtual ~Base() {};
	
	public:
		/**
		 * Tries to contsruct an object of a specialized class that represents
		 * the given datagram. If the datagram is valid but there is no specialized
		 * class, an object of class Generic is returned.
		 *
		 * If rawData is invalid and does not represent a ModBus telegram an
		 * exeption is thrown. E.g. rawDatagram might be invalid, because it is
		 * truncated or otherwise malformed
		 *
		 * @param rawDatagram The raw bytes
		 * @param length The length of the char array. I.e. it is NOT the length
		 * as given by length field of the ModBus telegram, but it is the length
		 * of the uninterpreted byte stream
		 */
		static Base* createObject( const char* rawDatagram, size_t length );
		
		
		/**
		 * Tries to read the length field out of rawDatagram and then returns
		 * how many bytes are missing to make rawDatagram a valid datagram.
		 * The length field in a ModBus datagram is located at bytes 5 and 6.
		 * Hence rawDatagram must be at least 6 bytes long.
		 *
		 * If rawDatagram is invalid an exception is thrown. The datagram might be
		 * invalid, because it is less than 6 bytes long or the bytes 1-4 do not
		 * match the ModBus specifications and therefore cannot be the beginning
		 * of a ModBus datagram
		 *
		 * @param rawDatagram The raw bytes
		 * @param length The length of the char array. I.e. it is NOT the length
		 * as given by length field of the ModBus telegram, but it is the length
		 * of the uninterpreted byte stream
		 */
		static size_t missingBytes( const char* rawDatagram, size_t length );

		/**
		 * According to ModBus specification a single datagram over TCP has at most
		 * 260 bytes.
		 * In former days a complete ModBus datagram (ADU = application data unit)
		 * over serial line (RS485) in RTU format was limited to 256 bytes. 3 bytes
		 * were needed by the header and error check code. Therefore 253 bytes 
		 * remained for the ModBus PDU. The ModBus header for TCP communication is
		 * 7 bytes, hence a full datagram has 253 + 7 = 260 bytes.
		 */
		enum { maxDatagramLength = 260 };

	public:
		/**
		 * @returns A new object of the same type that equals this object. The
		 * ownership of the new object is passed to the caller, i.e. it is the
		 * callers responsibility to free the memory
		 */
		virtual Base* copy() const = 0;
		
		/**
		 * @return The legth of the ModBus datagram as stored in the ModBus length
		 * field. This means the number of bytes in the function data field plus 2
		 * bytes
		 */
		virtual size_t length() const = 0;
		
		/**
		 * @return The total number of bytes that are required to store or transmit
		 * the datagram. I.e. the length of the ModBus datagram with the ModBus
		 * header included. This is supposed to equal Base::length() + 6.
		 */
		virtual size_t totalLength() const = 0;
		
		virtual FuncCode funcCode() const = 0;
		
		virtual TransactionID transactionID() const = 0;
		
		virtual void transactionID( TransactionID id ) = 0;
		
		virtual UnitID unitID() const = 0;
		
		virtual void unitID( UnitID id ) = 0;
		
		/**
		 * @return A pointer to the start address of the raw sequence of bytes. The
		 * number of bytes equals Base::totalLength().
		 */
		virtual const char* rawDatagram() const = 0;
};


} // namespace Datagram
} // namespace ModBus

#endif

#ifndef _MB_TYPEDEFS_H_
#define _MB_TYPEDEFS_H_

#include <sys/types.h>

namespace ModBus {
namespace Datagram {

typedef u_int16_t TransactionID;
typedef u_int8_t UnitID;
typedef u_int8_t FuncCode;
typedef u_int8_t ExceptionCode;

// The low level representation of a ModBus telegram header
// The encoding is expected to be in big endian as required by networks
// standards.
//
// Thus multi-byte value as MBHeader::transactionID, MBHeader::length and so on
// might seem as bogus on system with little endian byte order.
//
// This structure is not supposed to be used by other function and classes than
// those within the ModBus namespace.
struct Header {
	TransactionID transactionID;
	u_int16_t protocolID;
	u_int16_t length;    // length of funcData + 2 (includes the size of unitID and funcCode)
	UnitID unitID;
	FuncCode funcCode;
};

} // namespace Datagram
} // namespace ModBus

#endif

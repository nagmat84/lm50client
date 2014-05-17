#include "lib/mb_uint32.h"

namespace ModBus {
namespace Interpreter {

template< class DatagramClass > uint32_t UInt32< DatagramClass >::value( size_t index ) const {
	if( index >= N ) throw std::out_of_range( "index exceeds size of 32-bit integer array" );
	
	// The byte order in the datagram is big endian (network standard), thus
	// high order bytes come first. First a big-endian encoded integer is
	// constructed and then converted into correct endianess for the host
	
	u_int32_t bigEndian( ( reinterpret_cast< const u_int32_t* >( dgram.values() ) )[ index ] );
	
	return ntohl( bigEndian );
}

template class UInt32< Datagram::ReadHoldingRegistersRes >;
template class UInt32< Datagram::ReadInputRegistersRes >;

}
}

#include "lib/mb_uint16.h"

namespace ModBus {
namespace Interpreter {

template< class DatagramClass > uint16_t UInt16< DatagramClass >::value( size_t index ) const {
	if( index >= N ) throw std::out_of_range( "index exceeds size of 16-bit integer array" );
	
	// The byte order in the datagram is big endian (network standard), thus
	// high order bytes come first. First a big-endian encoded integer is
	// constructed and then converted into correct endianess for the host
	
	u_int16_t bigEndian( ( reinterpret_cast< const u_int16_t* >( dgram.values() ) )[ index ] );
	
	return ntohs( bigEndian );
}

template class UInt16< Datagram::ReadHoldingRegistersRes >;
template class UInt16< Datagram::ReadInputRegistersRes >;

}
}

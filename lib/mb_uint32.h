#ifndef _MB_UINT32_H_
#define _MB_UINT32_H_

#include "mb_rhregres.h"
#include "mb_riregres.h"

namespace ModBus {
namespace Interpreter {

template< class DatagramClass > class UInt32 {
	public:
		UInt32( const DatagramClass& datagram ) : dgram( datagram ), N( datagram.byteCount() / 4 ) {
			// The number of bytes in the datagram must be dividable by 4
			if( dgram.byteCount() & 3 ) throw std::invalid_argument( "datagram cannot converted into 32-bit integer due to incompatible number of bytes" );
		}
		
		virtual ~UInt32() {}
		
	public:
		size_t size() const {
			return N;
		}
		
		u_int32_t value( size_t index ) const;
		
	protected:
		const DatagramClass& dgram;
		const size_t N;
};

}
}

#endif

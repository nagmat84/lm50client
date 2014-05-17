#ifndef _MB_UINT16_H_
#define _MB_UINT16_H_

#include "mb_rhregres.h"
#include "mb_riregres.h"

namespace ModBus {
namespace Interpreter {

template< class DatagramClass > class UInt16 {
	public:
		UInt16( const DatagramClass& datagram ) : dgram( datagram ), N( datagram.byteCount() / 2 ) {
			// The number of bytes in the datagram must be dividable by 2
			if( dgram.byteCount() & 1 ) throw std::invalid_argument( "datagram cannot converted into 16-bit integer due to incompatible number of bytes" );
		}
		
		virtual ~UInt16() {}
		
	public:
		size_t size() const {
			return N;
		}
		
		u_int16_t value( size_t index ) const;
		
	protected:
		const DatagramClass& dgram;
		const size_t N;
};

}
}

#endif

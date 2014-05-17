#ifndef _MB_ASCII_H_
#define _MB_ASCII_H_

#include <string>
#include "mb_rhregres.h"
#include "mb_riregres.h"

namespace ModBus {
namespace Interpreter {

template< class DatagramClass > class ASCII {
	public:
		ASCII( const DatagramClass& datagram ) : dgram( datagram ) {}
		virtual ~ASCII() {}
		
	public:
		std::string string() const;
		operator std::string() const { return string(); }
		
	protected:
		const DatagramClass& dgram;
};

}
}

#endif

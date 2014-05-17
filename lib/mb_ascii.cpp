#include "lib/mb_ascii.h"

#include <sstream>

namespace ModBus {
namespace Interpreter {

template< class DatagramClass > std::string ASCII< DatagramClass >::string() const {
	std::ostringstream strstream;
	for( unsigned int j = 0; j < dgram.byteCount() && dgram.value(j) != '\0'; j++ ) strstream << dgram.value(j);
	return strstream.str();
}

template class ASCII< Datagram::ReadHoldingRegistersRes >;
template class ASCII< Datagram::ReadInputRegistersRes >;

}
}

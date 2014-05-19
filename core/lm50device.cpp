#include "lm50device.h"

namespace LM50 {

using namespace boost::posix_time;
using namespace ModBus;
using namespace std;

// Normally devices within the ModBus protocoll are addressed and identified
// by the unit id. For example this is true for "real" ModBus on a RS-432 line.
// For ModBus over TCP/IP the unit id has no meaning, because the device is
// already identified by its IP address but nonetheless a unit id is required
// to form a valid datagram. The LM50TCP+ requires the unit id to be 1.
const LM50Device::UnitId LM50Device::_unitId(1);


// The hardware revision of the LM-50TCP+ is stored in a "holding" register.
// The hardware revision is an ASCII string with at most 6 bytes (i.e. at most
// 5 letters and a trailing EOS). The ASCII characters are located at three
// 16-bit registers _hwAddrRevision, h_wAddrRevision+1 and _hwAddrRevision+2
const LM50Device::HwAddr   LM50Device::_hwAddrRevision( 0x0578 );
const LM50Device::HwLength LM50Device::_hwLengthRevision( 3 );

// The serial number of the LM-50TCP+ is stored in a "holding" register. The 
// serial number is a 32-bit unsigned integer and is located at the two 16-bit
// registers _hwAddrSerialNo and _hwAddrSerialNo+1.
const LM50Device::HwAddr   LM50Device::_hwAddrSerialNo( 0x2710 );
const LM50Device::HwLength LM50Device::_hwLengthSerialNo( 2 ) ;

// The LM50TCP+ has 50 channels that are stored in "input" registers. Each
// channel has a 32-bit unsigned integer value.
// The first value (32-bit) is located at 0x0080 and 0x0081
// The 50th value (32-bit)  is located at 0x00e2 and 0x00e3
// This means starting with adress 0x0080 there are 100 16-bit registers to
// read
const LM50Device::ChIdx    LM50Device::countChannels( 50 );
const LM50Device::HwAddr   LM50Device::_hwAddrChannels( 0x0080 );
const LM50Device::HwLength LM50Device::_hwLengthChannels( 100 );


LM50Device::LM50Device() : \
	_host(), \
	_port(), \
	_lastUpdate( not_a_date_time ), \
	_tcpComm(), \
	_lastRequestId(0), \
	_lastReplyId(0), \
	_revision(), \
	_serialNo(0), \
	_channels( new unsigned int[countChannels] ) {
	for( ChIdx i( 0 ); i < countChannels; ++i ) _channels[ i ] = 0;
}

LM50Device::LM50Device( const std::string& h, const std::string& p ) : \
	_host( h ), \
	_port( p ), \
	_lastUpdate( not_a_date_time ), \
	_tcpComm(), \
	_lastRequestId(0), \
	_lastReplyId(0), \
	_revision(), \
	_serialNo(0), \
	_channels( new unsigned int[countChannels] ) {
	for( ChIdx i( 0 ); i < countChannels; ++i ) _channels[ i ] = 0;
}

/**
 * Internal convinience function. It takes a request datagram, binds it to
 * the communication socket, sends it and waits for a respone datagram. The
 * primary purpose is to check for some common error conditions and reduce
 * code duplication
 * 
 * @param req The request datagram. The function assumes that this is either
 * a ReadHoldingRegistersReq or ReadInputRegistersReq object. No check is
 * performed, because this is an internal class function
 * @return Returns the response datagram. The return value is never NULL and
 * either a ReadHoldingRegistersRes or ReadInputRegistersRes object depending
 * on the type of the input parameter. The caller is responsible to free the
 * returned object.
 */
Datagram::Base* LM50Device::readValue( const Datagram::Base& req ) {
	TcpRequestAndReply rar( _tcpComm.createRequestAndReply( req ) );
	rar.run();
	const Datagram::Base* res = rar.response();
	if( res == nullptr ) throw runtime_error( "ModBus error: Time out" );
	const Datagram::ErrorRes* err = dynamic_cast< const Datagram::ErrorRes* >( res );
	if( err ) {
		_lastReplyId = err->transactionID();
		throw runtime_error( string( "ModBus error: " ).append( err->exceptionMessage() ) );
	}
	return rar.releaseResponse();
}

/**
 * Internal convinience function to read the value of a "holding" register from
 * the device. It calls LM50Device::readValue internally.
 * @param addr The starting address of the register
 * @param length The length of the register in 16bit blocks
 * @return Returns the response diagram. The return value is never NULL otherwise
 * an exception had been thrown. The caller is responsible to free the
 * returned object.
 */
Datagram::ReadHoldingRegistersRes* LM50Device::readHValue( HwAddr addr, HwLength length ) {
	Datagram::ReadHoldingRegistersReq req( ++_lastRequestId, _unitId, addr, length );
	// The return value of the next function call is not NULL and not an error
	// datagram, otherwise an exception had been thrown
	Datagram::Base* res = readValue( req );
	Datagram::ReadHoldingRegistersRes* hres = dynamic_cast< Datagram::ReadHoldingRegistersRes* >( res );
	if( !hres ) {
		delete res;
		throw runtime_error( "ModBus error: Unexepected datagram format" );
	}
	_lastReplyId = hres->transactionID();
	if( _lastReplyId != _lastRequestId ) {
		delete hres;
		throw runtime_error( "ModBus error: Received a response datagram, but it does not belong to our conversation" );
	}
	return hres;
}

/**
 * Internal convinience function to read the value of an "input" register from
 * the device. It calls LM50Device::readValue internally.
 * @param addr The starting address of the register
 * @param length The length of the register in 16bit blocks
 * @return Returns the response diagram. The return value is never NULL otherwise
 * an exception had been thrown. The caller is responsible to free the
 * returned object.
 */
Datagram::ReadInputRegistersRes* LM50Device::readIValue( HwAddr addr, HwLength length ) {
	Datagram::ReadInputRegistersReq req( ++_lastRequestId, _unitId, addr, length );
	// The return value of the next function call is not NULL and not an error
	// datagram, otherwise an exception had been thrown
	Datagram::Base* res = readValue( req );
	Datagram::ReadInputRegistersRes* ires = dynamic_cast< Datagram::ReadInputRegistersRes* >( res );
	if( !ires ) {
		delete res;
		throw runtime_error( "ModBus error: Unexepected datagram format" );
	}
	_lastReplyId = ires->transactionID();
	if( _lastReplyId != _lastRequestId ) {
		delete ires;
		throw runtime_error( "ModBus error: Received a response datagram, but it does not belong to our conversation" );
	}
	return ires;
}


/**
 * The LM50TCP+ has two holding registers that normally stay constant. One
 * register holds the hardware revision, the other stores the serial number.
 * This function queries their values and stores them in the respective
 * attributes of this class.
 */
void LM50Device::readSteadyValues() {
	// See comment on constants _hwAddrRevision, _hwLengthRevision,
	// _hwAddrSerialNo and _hwLengthSerialNo for further information.
	
	Datagram::ReadHoldingRegistersRes* hres = readHValue( _hwAddrRevision, _hwLengthRevision  );
	Interpreter::ASCII< Datagram::ReadHoldingRegistersRes > ascii( *hres );
	_revision = ascii.string();
	delete hres;
	
	hres = readHValue( _hwAddrSerialNo, _hwLengthSerialNo  );
	Interpreter::UInt32< Datagram::ReadHoldingRegistersRes > serial( *hres );
	_serialNo = serial.value( 0 );
	delete hres;
}

/**
 * The LM50TCP+ has 50 input registers that store the counter of each channels.
 * This function queries all 50 registers and writes their values into the
 * respective attributes of this class. On success the update time is
 * set to the current timestamp (@see LM50Device::lasUpdate).
 * Even if only one channel was needed it is cheap to query all 50 channels.
 * Most time is spent for establishing a TCP connection and the three-way
 * handshake. If more than one input register is needed, it is cheaper to
 * query all 50 input register in a bulk than query two registers individually,
 * even if the latter reuse an existing TCP connection. The LM50TCP+ seems be
 * very slow in interpreting a datagram, hence two datagrams are always
 * slower than one datagram, even if the latter has 50 times more payload.
 */
void LM50Device::updateVolatileValues() {
	// See comment on constants _hwAddrChannels and _hwLengthChannels for
	// further information.
	Datagram::ReadInputRegistersRes* ires = readIValue( _hwAddrChannels, _hwLengthChannels  );
	Interpreter::UInt32< Datagram::ReadInputRegistersRes > chs( *ires );
	if( chs.size() != countChannels ) {
		delete ires;
		throw runtime_error( "ModBus error: Received a truncated response datagram" );
	}
	for( ChIdx j = 0; j < countChannels; j++ ) _channels[j] = chs.value(j);
	delete ires;
	_lastUpdate = boost::posix_time::microsec_clock::universal_time();
}

/**
 * @return The hardware revision of the LM50TCP+
 * @throw runtime_error Thrown, if LM50Device::readSteadyValues has not been
 * called yet and the cached value is invalid.
 */
const string& LM50Device::revision() const {
	if( _revision.empty() ) throw runtime_error( "Revision value not valid. Function \"readSteadyValues\" has never been called yet or has not been successful" );
	return _revision;
}

/**
 * @return The serial number of the LM50TCP+
 * @throw runtime_error Thrown, if LM50Device::readSteadyValues has not been
 * called yet and the cached value is invalid.
 */
unsigned int LM50Device::serialNumber() const {
	if( _revision.empty() ) throw runtime_error( "Serial number not valid. Function \"readSteadyValues\" has never been called yet or has not been successful" );
	return _serialNo;
}

/**
 * @return The cached value of the given channel
 * @param ch The channel index between 0 and LM50Device::countChannels
 * @throw runtime_error Thrown, if LM50Device::updateVolatileValues has not been
 * called yet and the cached value is invalid.
 * @throw range_error Thrown, if parameter ch is out of range
 */
unsigned int LM50Device::channel( ChIdx ch ) const {
	if( _lastUpdate.is_not_a_date_time() ) throw runtime_error( "Channel values are not valid. Function \"updateVolatileValues\" has never been called yet or has not been successful" );
	if( ch >= countChannels ) {
		ostringstream msg;
		msg << "Channel index out of range. Index must be between 0 and " << (countChannels-1) << ".";
		throw range_error( msg.str() );
	}
	return _channels[ ch ];
}

}

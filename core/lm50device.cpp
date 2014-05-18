#include "program_options.h"

namespace LM50 {
	
using namespace boost::posix_time;
using namespace ModBus;
using namespace std;


const LM50Device::UnitId LM50Device::_unitId(1);
const LM50Device::ChIdx  LM50Device::firstChannel(1);
const LM50Device::ChIdx  LM50Device::lastChannel(50);
const LM50Device::HwAddr LM50Device::hwAddrRevision( 0x0578 );
const LM50Device::HwAddr LM50Device::hwAddrSerialNo( 0x2710 );
const LM50Device::HwAddr LM50Device::_hwAddrChannel[ LM50Device::lastChannel ] = {
	0x0080, 0x0082, 0x0084, 0x0086, 0x0088, 0x008a, 0x008c, 0x008e,
	0x0090, 0x0092, 0x0094, 0x0096, 0x0098, 0x009a, 0x009c, 0x009e,
	0x00a0, 0x00a2, 0x00a4, 0x00a6, 0x00a8, 0x00aa, 0x00ac, 0x00ae,
	0x00b0, 0x00b2, 0x00b4, 0x00b6, 0x00b8, 0x00ba, 0x00bc, 0x00be,
	0x00c0, 0x00c2, 0x00c4, 0x00c6, 0x00c8, 0x00ca, 0x00cc, 0x00ce,
	0x00d0, 0x00d2, 0x00d4, 0x00d6, 0x00d8, 0x00da, 0x00dc, 0x00de,
	0x00e0, 0x00e2
};


LM50Device::LM50Device() : \
	_host(), \
	_port(), \
	_lastUpdate( not_a_date_time ), \
	_tcpComm(), \
	_lastRequestId(0), \
	_lastReplyId(0), \
	_revision(), \
	_serialNo(0) {
	for( ChIdx i( firstChannel ); i <= lastChannel; ++i ) _channels[ i-firstChannel ] = 0;
}

LM50Device::LM50Device( const std::string& h, const std::string& p ) : \
	_host( h ), \
	_port( p ), \
	_lastUpdate( not_a_date_time ), \
	_tcpComm(), \
	_lastRequestId(0), \
	_lastReplyId(0), \
	_revision(), \
	_serialNo(0) {
	for( ChIdx i( firstChannel ); i <= lastChannel; ++i ) _channels[ i-firstChannel ] = 0;
}

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


void LM50Device::readSteadyValues() {
	// Read hardware revision of the LM-50TCP+. This is a ASCII string with
	// at most 6 bytes (i.e. at most 5 letters and a trailing EOS).
	// The ASCII characters are located at the the three 16-bit registers
	// hwAddrRevision, hwAddrRevision+1 and hwAddrRevision+2
	Datagram::ReadHoldingRegistersRes* hres = readHValue( hwAddrRevision, 3  );
	Interpreter::ASCII< Datagram::ReadHoldingRegistersRes > ascii( *hres );
	_revision = ascii.string();
	delete hres;
	
	// Read serial number of the LM-50TCP+. This is a 32-bit unsigned integer
	// and is located at the two 16-bit registers hwAddrSerialNo and hwAddrSerialNo+1.
	hres = readHValue( hwAddrSerialNo, 2  );
	Interpreter::UInt32< Datagram::ReadHoldingRegistersRes > serial( *hres );
	_serialNo = serial.value( 0 );
	delete hres;
}

void LM50Device::updateVolatileValues() {
	// Now read all meter values
	// The first value (32-bit) is located at 0x0080 and 0x0081
	// The 50th value (32-bit)  is located at 0x00e2 and 0x00e3
	// This means starting with adress 0x0080 there are 100 16-bit registers to
	// read
	Datagram::ReadInputRegistersRes* ires = readIValue( _hwAddrChannel[0], 2*(lastChannel - firstChannel + 1)  );
	Interpreter::UInt32< Datagram::ReadInputRegistersRes > chs( *ires );
	if( chs.size() != lastChannel - firstChannel + 1 ) {
		delete ires;
		throw runtime_error( "ModBus error: Received a truncated response datagram" );
	}
	for( ChIdx j = 0; j < chs.size(); j++ ) _channels[j] = chs.value(j);
	delete ires;
	_lastUpdate = boost::posix_time::microsec_clock::universal_time();
}

const string& LM50Device::revision() const {
	if( _revision.empty() ) throw runtime_error( "Revision value not valid. Function \"readSteadyValues\" has never been called yet or has not been successful" );
	return _revision;
}

unsigned int LM50Device::serialNumber() const {
	if( _revision.empty() ) throw runtime_error( "Serial number not valid. Function \"readSteadyValues\" has never been called yet or has not been successful" );
	return _serialNo;
}

unsigned int LM50Device::channel( ChIdx ch ) const {
	if( _lastUpdate.is_not_a_date_time() ) throw runtime_error( "Channel values are not valid. Function \"updateVolatileValues\" has never been called yet or has not been successful" );
	if( firstChannel > ch || ch > lastChannel ) {
		ostringstream msg;
		msg << "Channel index out of range. Index must be between " << firstChannel << " and " << lastChannel << ".";
		throw range_error( msg.str() );
	}
	return _channels[ ch - firstChannel ];
}

}

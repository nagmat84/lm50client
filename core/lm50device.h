#ifndef _LM50DEVICE_H_
#define _LM50DEVICE_H_

#include <cstddef>
#include <sys/types.h>
#include <boost/date_time.hpp>

#include "lib/modbus.h"

namespace LM50 {

/**
 * This class describes a LM50d device
 */
class LM50Device {
	public:
		typedef ModBus::Datagram::UnitID UnitId;
		typedef ModBus::Datagram::TransactionID TransactionId;
		typedef size_t ChIdx;
		typedef uint16_t HwAddr;
		typedef uint16_t HwLength;
	
	public:
		LM50Device();
		LM50Device( const std::string& h, const std::string& p );
		virtual ~LM50Device() {}
		
	public:
		static HwAddr hwAddrChannel( ChIdx ch ) {
			assert( ch >= firstChannel && ch <= lastChannel );
			return _hwAddrChannel[ch-firstChannel];
		}
		
		const std::string& host() const { return _host; }
		
		void host( std::string h ) { _host = h; }
		
		const std::string& port() const { return _port; }
		
		void port( std::string p ) { _port = p; }
		
		void connect() { _tcpComm.open( _host, _port ); }
		
		void disconnect() { _tcpComm.close(); }
		
		void readSteadyValues();
		
		void updateVolatileValues();
		
		const std::string& revision() const;
		
		unsigned int serialNumber() const;
		
		unsigned int channel( ChIdx ch ) const;
		
		const boost::posix_time::ptime& lastUpdate() const { return _lastUpdate; }
		
	protected:
		ModBus::Datagram::Base* readValue( const ModBus::Datagram::Base& req );
		ModBus::Datagram::ReadHoldingRegistersRes* readHValue( HwAddr addr, HwLength length );
		ModBus::Datagram::ReadInputRegistersRes* readIValue( HwAddr addr, HwLength length );
		
	public:
		static const ChIdx firstChannel;
		static const ChIdx lastChannel;
		static const HwAddr hwAddrRevision;
		static const HwAddr hwAddrSerialNo;
		
	protected:
		static const UnitId _unitId;
		static const HwAddr _hwAddrChannel[ 50 ];
		std::string _host;
		std::string _port;
		boost::posix_time::ptime _lastUpdate;
		ModBus::TcpCommunication _tcpComm;
		TransactionId _lastRequestId;
		TransactionId _lastReplyId;
		std::string _revision;
		unsigned int _serialNo;
		unsigned int _channels[ 50 ];
};

}

#endif

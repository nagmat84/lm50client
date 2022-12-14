#ifndef _LM50DEVICE_H_
#define _LM50DEVICE_H_

#include <cstddef>
//include <sys/types.h>
#include <ctime>

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
		typedef unsigned int ChVal;
		typedef uint16_t HwAddr;
		typedef uint16_t HwLength;
	
	public:
		LM50Device();
		LM50Device( const std::string& h, const std::string& p );
		virtual ~LM50Device() { delete[] _channels; }
		
	public:
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
		
		ChVal channel( ChIdx ch ) const;
		
		const ChVal* channels() const { return _channels; }
		
		const struct timespec& lastUpdate() const { return _lastUpdate; }
		
	protected:
		/*static HwAddr hwAddrChannel( ChIdx ch ) {
			assert( ch >= firstChannel && ch <= lastChannel );
			return _hwAddrChannel[ch-firstChannel];
		}*/
		ModBus::Datagram::Base* readValue( const ModBus::Datagram::Base& req );
		ModBus::Datagram::ReadHoldingRegistersRes* readHValue( HwAddr addr, HwLength length );
		ModBus::Datagram::ReadInputRegistersRes* readIValue( HwAddr addr, HwLength length );
		
	public:
		//static const ChIdx firstChannel;
		static const ChIdx countChannels;
		
	protected:
		static const UnitId   _unitId;
		static const HwAddr   _hwAddrRevision;
		static const HwLength _hwLengthRevision;
		static const HwAddr   _hwAddrSerialNo;
		static const HwLength _hwLengthSerialNo;
		static const HwAddr   _hwAddrChannels;
		static const HwLength _hwLengthChannels;
		std::string _host;
		std::string _port;
		struct timespec _lastUpdate;
		ModBus::TcpCommunication _tcpComm;
		TransactionId _lastRequestId;
		TransactionId _lastReplyId;
		std::string _revision;
		unsigned int _serialNo;
		ChVal* _channels;
};

}

#endif

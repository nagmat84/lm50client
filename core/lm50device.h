#ifndef _LM50DEVICE_H_
#define _LM50DEVICE_H_

#include <cstddef>
#include <sys/types.h>

namespace LM50 {

/**
 * This class describes a LM50d device
 */
class LM50Device {
	public:
		typedef size_t ChIdx;
		typedef uint16_t HwAddr;
	
	public:
		LM50Device() {}
		virtual ~LM50Device() {}
		
	public:
		static HwAddr hwAddrChannel( size_t channel ) {
			assert( channel >= firstChannel && channel <= lastChannel );
			return _hwAddrChannel[channel-firstChannel];
		}
		
	public:
		static const ChIdx firstChannel;
		static const ChIdx lastChannel;
		static const HwAddr hwAddrVersion;
		static const HwAddr hwAddrSerial;
		
	protected:
		static const HwAddr _hwAddrChannel[ 50 ];
};

}

#endif

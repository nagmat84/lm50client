#ifndef _MB_CONSTANTS_H_
#define _MB_CONSTANTS_H_

//include <sys/types.h>
//include <netinet/in.h>
//include <cassert>

#include "lib/mb_typedefs.h"

namespace ModBus {

/**
 * Well defined function codes. Some ModBus terminology: ModBus
 * distinguishes between four different types of quantities.
 * 
 * The first
 * criterion is, how the quantity can be altered. There are quantities that
 * are related to some physical input into the device (e.g.. measurement).
 * Hence they can change over time, if the physical parameter changes,
 * but they are read-only from the program's perspective. On the other hand
 * side there are quantities that are directly stored in the device itsself
 * and therefore can be written and read from the program.
 * 
 * The second criterion is the quantity's range. In ModBus there are
 * exactly two types: 1-bit registers and 16-bit registers. Each register
 * no matter if 1- or 16-bit is identified by its address, which it is
 * also a 16-bit integer. If one need different types, for example a
 * 32-bit value, one must combine several registers, for example two
 * 16-bit registers. But this construction has to be done in the
 * application. ModBus is totally unaware of this and does not know
 * if two or more registers belong together or not.
 * 
 *   - "discrete input"   is a 1-bit register that depends on a physical
 *                        input, i.e. is read-only
 *   - "coil"             is a 1-bit register that provides write and read
 *                        access
 *   - "input register"   is a 16-bit register that depens on a physical
 *                        input, i.e. is read-only
 *   - "holding register" is a 16-bit register that provides write and read
 *                        access
 */
namespace Function {
	enum Function {
		ReadCoils = 0x01,
		ReadDiscreteInputs = 0x02,
		ReadHoldingRegisters = 0x03,
		ReadInputRegisters = 0x04,
		WriteSingleCoil = 0x05,
		WriteSingleRegister = 0x06,
		WriteMultipleCoils = 0x0f,
		WriteMultipleRegisters = 0x10,
		ReadFileRecord = 0x14,
		WriteFileRecord = 0x15,
		MaskWriteRegister = 0x16,
		ReadWriteMultipleRegisters = 0x17,
		ReadFIFOQueue = 0x18,
		ReadDeviceIdent = 0x2b,
		Error = 0x80
	} __attribute__ ((__packed__));
}



/**
 * Well defined exception codes. These are numeric constants are used by a
 * device in a reply to indicate an error
 */
namespace Exception {
	enum Exception {
		IllegalFunction  = 0x01,
		IllegalAddress = 0x02,
		IllegalValue = 0x03,
		DeviceFailure = 0x04,
		Acknowledge = 0x05,
		DeviceBusy = 0x06,
		ParityError = 0x08,
		GatewayUnavailable = 0x0a,
		TargetFailure = 0x0b
	} __attribute__ ((__packed__));
}

} // namespace ModBus

#endif

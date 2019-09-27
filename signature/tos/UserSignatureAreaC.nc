/**
 * DeviceSignature initialization during TinyOS boot process.
 *
 * @author Raido Pahtma
 * @license MIT
 */
#include "DeviceSignature.h"
#include "checksum.h"
#include "crc.h"
module UserSignatureAreaC {
	provides {
		interface Boot;
		interface LocalIeeeEui64;
	}
	uses {
		interface Boot as SubBoot;
	}
}
implementation {

	#define __MODUUL__ "usig"
	#define __LOG_LEVEL__ ( LOG_LEVEL_UserSignatureAreaC & BASE_LOG_LEVEL )
	#include "log.h"

	event void SubBoot.booted()
	{
		if(SIG_GOOD != sigInit())
		{
			err1("sig BAD");
		}
		signal Boot.booted();
	}

	command ieee_eui64_t LocalIeeeEui64.getId()
	{
		ieee_eui64_t eui;
		sigGetEui64((uint8_t*)&eui);
		return eui;
	}

	uint16_t update_crc_ccitt(uint16_t crc, unsigned char b) @C() @spontaneous() {
		return crcByte(crc, b);
	}

}

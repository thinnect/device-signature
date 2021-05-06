/**
 * SiLabs UserData area API implementation.
 *
 * @author Raido Pahtma
 * @license MIT
 */
#include "em_device.h"

#include <string.h> // memcpy

/*
 * Return the size of the signature area.
 */
uint16_t sigAreaGetSize ()
{
	return USERDATA_SIZE;
}

/*
 * Read from signature area, return number of bytes read.
 */
uint16_t sigAreaRead (uint16_t offset, void* buf, uint16_t length)
{
	memcpy(buf, ((void*)USERDATA_BASE)+offset, length);
	return length;
}

/*
 * Read byte from signature area, output undefined,
 * if offset is outside of signature.
 */
uint8_t sigAreaReadByte (uint16_t offset)
{
	return *((uint8_t*)(((void*)USERDATA_BASE) + offset));
}

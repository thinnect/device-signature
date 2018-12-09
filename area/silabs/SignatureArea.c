/**
 * SiLabs UserData area API implementation.
 *
 * @author Raido Pahtma
 * @license MIT
 */

#include <stdint.h>
#include <string.h> // memcpy

// Mighty Gecko parameters
#define USERSIG_ADDR (void*)0x0FE00000
#define USER_PAGE_SIZE 2048

/*
 * Return the size of the signature area.
 */
uint16_t sigAreaGetSize() {
	return USER_PAGE_SIZE;
}

/*
 * Read from signature area, return number of bytes read.
 */
uint16_t sigAreaRead(uint16_t offset, void* buf, uint16_t length) {
	memcpy(buf, USERSIG_ADDR+offset, length);
	return length;
}

/*
 * Read byte from signature area, output undefined,
 * if offset is outside of signature.
 */
uint8_t sigAreaReadByte(uint16_t offset) {
	return *((uint32_t*)(USERSIG_ADDR + offset));
}

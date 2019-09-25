/**
 * Atmel RFR2 UserData area API implementation.
 *
 * @author Raido Pahtma
 * @license MIT
 */

#include <stdint.h>
#include <string.h> // memcpy

#include <avr/boot.h>
#include <avr/interrupt.h>

// Atmel RFR2 parameters
#define USERSIG_ADDR 0x100
#define USERSIG_SIZE 768

/*
 * Return the size of the signature area.
 */
uint16_t sigAreaGetSize() {
	return USERSIG_SIZE;
}

/*
 * Read from signature area, return number of bytes read.
 */
uint16_t sigAreaRead(uint16_t offset, void* buf, uint16_t length) {
	uint16_t i;
	uint8_t sreg = SREG; // Back up SREG before disabling interrupts

	if(offset + length > USERSIG_SIZE) {
		length = USERSIG_SIZE - (uint16_t)offset;
	}

	// Disable interrupts
	cli();
	asm volatile("" : : : "memory");

	// Read the signature bytes while interrupts are disabled
	for(i=0;i<length;i++) {
		((uint8_t*)buf)[i] = boot_signature_byte_get(USERSIG_ADDR + offset + i);
	}

	// Restore SREG (possibly enable interrupts)
	asm volatile("" : : : "memory");
	SREG = sreg;

	return i;
}

/*
 * Read byte from signature area.
 * Output undefined, if offset is outside of signature.
 */
uint8_t sigAreaReadByte(uint16_t offset) {
	uint8_t b;

	sigAreaRead(offset, &b, sizeof(b));

	return b;
}

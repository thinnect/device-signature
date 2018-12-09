/**
 * Device user signature / user data area access API.
 *
 * @author Raido Pahtma
 * @license MIT
 */
#ifndef SIGNATUREAREA_H_
#define SIGNATUREAREA_H_

#include <stdint.h>

/*
 * Return the size of the signature area.
 */
uint16_t sigAreaGetSize();

/*
 * Read from signature area, return number of bytes read.
 */
uint16_t sigAreaRead(uint16_t offset, void* buf, uint16_t length);

/*
 * Read byte from signature area, output undefined,
 * if offset is outside of signature.
 */
uint8_t  sigAreaReadByte(uint16_t offset);

#endif//SIGNATUREAREA_H_

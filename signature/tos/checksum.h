/**
 * DeviceSignature normally uses CRC function from the checksum.h in
 * https://github.com/lammertb/libcrc, however the necessary CRC function is
 * always present in TinyOS and therefore a wrapper has been created to make
 * use of it and save space. The actual update_crc_ccitt body is in the
 * UserSignatureAreaC module as the TinyOS CRC function is not accessible
 * directly from a regular .c file.
 *
 * Copyright Thinnect Inc. 2019
 *
 * @author Raido Pahtma
 * @license MIT
 */
#ifndef CHECKSUM_H_
#define CHECKSUM_H_

#define CRC_START_XMODEM 0

uint16_t update_crc_ccitt(uint16_t crc, unsigned char b);

#endif//CHECKSUM_H_

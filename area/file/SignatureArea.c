/**
 * Signature from a specified file.
 *
 * @author Raido Pahtma
 * @license MIT
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char* sigFileName = NULL;

void sigAreaInit(const char* filename)
{
	if(sigFileName != NULL) {
		free(sigFileName);
	}

	if(filename != NULL) {
		int len = strlen(filename) + 1;
		sigFileName = (char*)malloc(len);
		if(sigFileName != NULL) {
			memcpy(sigFileName, filename, len);
		}
	}
}

/*
 * Return the size of the signature area.
 */
uint16_t sigAreaGetSize()
{
	size_t size = 0;
	FILE* f = fopen(sigFileName, "rb");
	if(f != NULL) {
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fclose(f);
	}
	return (uint16_t)size;
}

/*
 * Read from signature area, return number of bytes read.
 */
uint16_t sigAreaRead(uint16_t offset, void* buf, uint16_t length)
{
	uint16_t rd = 0;
	FILE* f = fopen(sigFileName, "rb");
	if(f != NULL) {
		fseek(f, offset, SEEK_SET);
		rd = fread(buf, length, 1, f);
		fclose(f);
	}
	return rd;
}

/*
 * Read byte from signature area, output undefined,
 * if offset is outside of signature.
 */
uint8_t sigAreaReadByte(uint16_t offset)
{
	uint8_t b = 0;
	sigAreaRead(offset, &b, 1);
	return b;
}

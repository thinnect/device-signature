#ifndef ENDIANESSY_H_
#define ENDIANESSY_H_

// Messy endianess:
// initial hack endianess management, need to figure out where to get
// sensible functions to cover all possible platforms.

//#include <endian.h>
//#include <byteswap.h>

uint16_t swap_uint16(uint16_t x)
{
	return ((x) >> 8) | ((x) << 8);
}

int64_t swap_int64(int64_t val)
{
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
}

uint64_t swap_uint64(uint64_t val)
{
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | (val >> 32);
}

#endif//ENDIANESSY_H_

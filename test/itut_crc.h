#ifndef ITUT_CRC_H_
#define ITUT_CRC_H_

#include <stdint.h>

uint16_t byteCrc(uint8_t b, uint16_t crc) {
    uint8_t i = 8;
    crc = crc ^ b << 8;
    do {
        if (crc & 0x8000) crc = crc << 1 ^ 0x1021;
        else crc = crc << 1;
    }
    while(--i);

    return crc;
}

uint16_t calculateCrc(const uint8_t ptr[], uint8_t length) {
    uint8_t i;
    uint16_t crc = 0;
    while(length-- > 0) {
        crc = crc ^ *ptr++ << 8;
        i = 8;
        do {
            if (crc & 0x8000) crc = crc << 1 ^ 0x1021;
            else crc = crc << 1;
        }
        while(--i);
    }
    return crc;
}

#endif//ITUT_CRC_H_

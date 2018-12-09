#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <stddef.h>
#ifndef offsetof
#define offsetof(t, m) ((size_t)&((t*)0)->m )
#endif//offsetof

#include "itut_crc.h"

#include "SignatureArea.h"

#include "DeviceSignature.h"
#include "DeviceSignatureTypes.h"

#include "endianessy.h"

static uint8_t mStatus = SIG_OFF;
static uint8_t mEUI64[8];

uint16_t findSignature(uint8_t sigtype, uint16_t offset)
{
	while(offset < sigAreaGetSize()) {
		uint16_t signSize = 0;
		packed_semver_t ver;
		uint8_t signType = 0;
		uint16_t i;

		uint16_t crc = 0;
		uint16_t scrc = 0;

		sigAreaRead(offset, &ver, sizeof(ver)); // Read signature version

		if (ver.major == 1) { // Ver 1 signatures have a fixed size
			signSize = 768;
			if(sigtype != 0) {
				return 0xFFFF; // Ver 1 only has sig type 0
			}
		}
		else { // Others specify the size after the version
			if ((ver.major == 2)&&(sigtype != 0)) {
				return 0xFFFF; // Ver 2 only has sig type 0
			}
			sigAreaRead(offset+sizeof(ver), &signSize, sizeof(ver));
			signSize = swap_uint16(signSize); // Values are stored big-endian
		}

		if (signSize == 0xFFFF) { // Probably version was FF.FF.FF as well
			return 0xFFFF; // Not found
		}

		if (signSize < sizeof(ver)+sizeof(signSize)+2) {
			printf("SIG size %d\n", signSize);
			return 0xFFFF; // Cannot continue searching
		}

		for (i=0; i<(signSize-2); i++) {
			crc = byteCrc(sigAreaReadByte(offset+i), crc); // TODO use a generic CRC function
		}
		sigAreaRead(offset+i, &scrc, 2);
		scrc = swap_uint16(scrc);

		if (crc != scrc) {
			printf("SIG ERR: %x != %x\n", scrc, crc);
			return 0xFFFF;
		}

		//printf("SIG@%04X OK CRC:%x\n", offset, scrc, crc);

		if ((ver.major == 1)||(ver.major == 2)) {
			return offset; // Got a good ver 1 or 2 signature, which are always type 0
		}

		sigAreaRead(offset+sizeof(ver)+sizeof(signSize), &signType, sizeof(signType));

		if(signType == sigtype) { // Found requested type of signature
			return offset;
		}

		offset += signSize;
	}
	return 0xFFFF;
}

semver_t getSignatureVersion(uint16_t offset) {
	semver_t v = {0,0,0};
	sigAreaRead(offset, &v, sizeof(v));
	return v;
}

void getEui64(uint8_t eui[8], uint16_t offset) {
	semver_t v = getSignatureVersion(offset);
	if(v.major == 1) {
		sigAreaRead(offset+offsetof(usersig_header_v1_t, eui64), eui, 8);
	}
	else if(v.major == 2) {
		sigAreaRead(offset+offsetof(usersig_header_v2_t, eui64), eui, 8);
	}
	else {
		sigAreaRead(offset+offsetof(usersig_header_v3_eui_t, eui64), eui, 8);
	}
}

int8_t sigInit(void)
{
	uint16_t offset = findSignature(0, 0);
	if (offset < 0xFFFF) {
		mStatus = SIG_GOOD;
		getEui64(mEUI64, offset);
	}
	else {
		memset(mEUI64, 0, sizeof(mEUI64));
		mStatus = SIG_EMPTY;
		for(uint16_t i=0;i<sigAreaGetSize();i++) { // Detect if uninitialized
			if(sigAreaReadByte(i) != 0xFF) {
				mStatus = SIG_BAD;
				break;
			}
		}
	}
	return mStatus;
}

void sigGetEui64(uint8_t *buf)
{
	memcpy(buf, mEUI64, 8);
}

uint16_t sigGetNodeId(void)
{
	return mEUI64[6] << 8 | mEUI64[7];
}

semver_t sigGetSignatureVersion(void)
{
	semver_t v = {0,0,0};
	uint16_t offset = findSignature(0, 0);
	if(offset < 0xFFFF) {
		return getSignatureVersion(0);
	}
	return v;
}

semver_t sigGetPcbVersion(void)
{
	packed_semver_t pv = {0,0,0};
	semver_t v;

	uint16_t offset = findSignature(SIGNATURE_TYPE_PLATFORM, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, component_version_major), &pv, sizeof(pv));
	}
	else { // Maybe no platform defined, fall back to board
		offset = findSignature(SIGNATURE_TYPE_BOARD, 0);
		if (offset < 0xFFFF) { // This is version 3 (or later)
			sigAreaRead(offset+offsetof(usersig_header_v3_component_t, component_version_major), &pv, sizeof(pv));
		}
		else {
			offset = findSignature(0, 0);
			if (offset < 0xFFFF) {
				semver_t sigver = getSignatureVersion(offset);
				if(sigver.major == 1) { // v1 does not have UUIDs
					sigAreaRead(offset+offsetof(usersig_header_v1_t, pcb_version_major), &pv, sizeof(pv));
				}
				else if(sigver.major == 2) {
					sigAreaRead(offset+offsetof(usersig_header_v2_t, pcb_version_major), &pv, sizeof(pv));
				}
			}
		}
	}

	v.major = pv.major;
	v.minor = pv.minor;
	v.patch = pv.patch;
	return v;
}

void sigGetBoardUUID(uint8_t uuid[16])
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_BOARD, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, component_uuid), uuid, USERSIG_UUID_LENGTH);
	}
	else { // version 1 or 2
		offset = findSignature(0, 0);
		if (offset < 0xFFFF) {
			semver_t sigver = getSignatureVersion(offset);
			if(sigver.major == 1) { // v1 does not have UUIDs
				memset(uuid, 0, USERSIG_UUID_LENGTH); // TODO implement the mapping
			}
			else if(sigver.major == 2) {
				sigAreaRead(offset+offsetof(usersig_header_v2_t, board_uuid), uuid, USERSIG_UUID_LENGTH);
			}
			else {
				memset(uuid, 0, USERSIG_UUID_LENGTH);
			}
		}
		else {
			memset(uuid, 0, USERSIG_UUID_LENGTH);
		}
	}
}

void sigGetPlatformUUID(uint8_t uuid[16])
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_PLATFORM, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, component_uuid), uuid, USERSIG_UUID_LENGTH);
	}
	else { // Fall back to board UUID for older versions or cases where there is no platform sig
		sigGetBoardUUID(uuid);
	}
}

void sigGetManufacturerUUID(uint8_t uuid[16])
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_PLATFORM, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, manufacturer_uuid), uuid, USERSIG_UUID_LENGTH);
	}
	else { // Fall back to board manufacturer if no platform manufacturer
		offset = findSignature(SIGNATURE_TYPE_BOARD, 0);
	 	if (offset < 0xFFFF) {
	 		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, manufacturer_uuid), uuid, USERSIG_UUID_LENGTH);
	 	}
	 	else { // version 1 or 2
			offset = findSignature(0, 0);
			if (offset < 0xFFFF) {
				semver_t sigver = getSignatureVersion(offset);
				if(sigver.major == 1) { // v1 does not have manufacturer info
					memset(uuid, 0, USERSIG_UUID_LENGTH);
				}
				else if(sigver.major == 2) {
					sigAreaRead(offset+offsetof(usersig_header_v2_t, manufacturer_uuid), uuid, USERSIG_UUID_LENGTH);
				}
				else {
					memset(uuid, 0, USERSIG_UUID_LENGTH);
				}
			}
			else {
				memset(uuid, 0, USERSIG_UUID_LENGTH);
			}
		}
	}
}

void sigGetBoardName(uint8_t buf[17])
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_BOARD, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, component_name), buf, 16);
	}
	else { // version 1 or 2
		offset = findSignature(0, 0);
		if (offset < 0xFFFF) {
			semver_t sigver = getSignatureVersion(offset);
			if(sigver.major == 1) {
				sigAreaRead(offset+offsetof(usersig_header_v1_t, board_name), buf, 16);
				buf[16] = '\0';
			}
			else if (sigver.major == 2) {
				sigAreaRead(offset+offsetof(usersig_header_v2_t, board_name), buf, 16);
				buf[16] = '\0';
			}
			else {
				buf[0] = '\0';
			}
		}
		else {
			buf[0] = '\0';
		}
	}
}

void sigGetPlatformName(uint8_t buf[17])
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_PLATFORM, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, component_name), buf, 16);
	}
	else { // Fall back to board name for older versions or cases where there is no platform sig
		sigGetBoardName(buf);
	}
}

int64_t sigGetProductionTime(void)
{
	int64_t timestamp = 0;
	uint16_t offset = findSignature(SIGNATURE_TYPE_PLATFORM, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, unix_time), &timestamp, sizeof(timestamp));
	}
	else { // version 1 or 2
		offset = findSignature(SIGNATURE_TYPE_BOARD, 0);
		if (offset < 0xFFFF) { // This is version 3 (or later)
			sigAreaRead(offset+offsetof(usersig_header_v3_component_t, unix_time), &timestamp, sizeof(timestamp));
		}
		else {
			offset = findSignature(0, 0);
			if (offset < 0xFFFF) {
				semver_t sigver = getSignatureVersion(offset);
				if(sigver.major == 1) {
					sigAreaRead(offset+offsetof(usersig_header_v1_t, unix_time), &timestamp, sizeof(timestamp));
				}
				else if(sigver.major == 2) {
					sigAreaRead(offset+offsetof(usersig_header_v2_t, unix_time), &timestamp, sizeof(timestamp));
				}
			}
		}
	}
	timestamp = swap_int64(timestamp);
	return timestamp;
}


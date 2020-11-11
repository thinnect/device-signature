#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <stddef.h>
#ifndef offsetof
#define offsetof(t, m) ((size_t)&((t*)0)->m )
#endif//offsetof

#include "checksum.h" // for crc_xmodem / crc_ccitt_generic from libcrc
#include "endianness.h"

#include "SignatureArea.h"

#include "DeviceSignature.h"
#include "DeviceSignatureTypes.h"

#include "platform_eui.h"


static uint8_t mStatus = SIG_OFF;
static uint8_t mEUI64[8];

static uint16_t findSignature(uint8_t sigtype, uint16_t offset)
{
	while (offset < sigAreaGetSize()) {
		uint16_t signSize = 0;
		packed_semver_t ver;
		uint8_t signType = 0;

		sigAreaRead(offset, &ver, sizeof(ver)); // Read signature version

		if (ver.major == 1) { // Ver 1 signatures have a fixed size
			signSize = 768;
			if (sigtype != 0) {
				return 0xFFFF; // Ver 1 only has sig type 0
			}
		}
		else { // Others specify the size after the version
			if ((ver.major == 2)&&(sigtype != 0)) {
				return 0xFFFF; // Ver 2 only has sig type 0
			}
			sigAreaRead(offset+sizeof(ver), &signSize, sizeof(signSize));
			signSize = ntoh16(signSize); // Values are stored big-endian
		}

		if (signSize == 0xFFFF) { // Probably version was FF.FF.FF as well
			return 0xFFFF; // Not found
		}

		if (signSize < sizeof(ver)+sizeof(signSize)+2) {
			//printf("SIG size %d\n", signSize);
			return 0xFFFF; // Cannot continue searching
		}

		#ifndef DEVICESIGNATURE_DISABLE_CRC
		{
			uint16_t crc = CRC_START_XMODEM;
			uint16_t scrc = 0;
			uint16_t i;
			for (i=0; i<(signSize-2); i++) {
				crc = update_crc_ccitt(crc, sigAreaReadByte(offset+i));
			}
			sigAreaRead(offset+i, &scrc, 2);
			scrc = ntoh16(scrc);

			if (crc != scrc) {
				//printf("SIG ERR: %x != %x\n", scrc, crc);
				return 0xFFFF;
			}

			//printf("SIG@%04X OK CRC:%x\n", offset, scrc, crc);
		}
		#endif//DEVICESIGNATURE_DISABLE_CRC

		if ((ver.major == 1)||(ver.major == 2)) {
			return offset; // Got a good ver 1 or 2 signature, which are always type 0
		}

		sigAreaRead(offset+sizeof(ver)+sizeof(signSize), &signType, sizeof(signType));

		if (signType == sigtype) { // Found requested type of signature
			return offset;
		}

		offset += signSize;
	}
	return 0xFFFF;
}

semver_t getSignatureVersion(uint16_t offset) {
	semver_t v = {0,0,0};
	packed_semver_t pv;
	sigAreaRead(offset, &pv, sizeof(pv));
	v.major = pv.major;
	v.minor = pv.minor;
	v.patch = pv.patch;
	return v;
}

uint16_t getSignatureLength(uint16_t offset)
{
	semver_t ver = getSignatureVersion(offset);
	if (ver.major == 1) { // Ver 1 signatures have a fixed size
		return 768;
	}
	else { // Others specify the size after the version
		uint16_t signSize;
		sigAreaRead(offset+sizeof(ver), &signSize, sizeof(signSize));
		return ntoh16(signSize); // Values are stored big-endian
	}
	return 0;
}

void getEui64(uint8_t eui[8], uint16_t offset) {
	semver_t v = getSignatureVersion(offset);
	if (v.major == 1) {
		sigAreaRead(offset+offsetof(usersig_header_v1_t, eui64), eui, 8);
	}
	else if (v.major == 2) {
		sigAreaRead(offset+offsetof(usersig_header_v2_t, eui64), eui, 8);
	}
	else {
		sigAreaRead(offset+offsetof(usersig_header_v3_eui_t, eui64), eui, 8);
	}
}

int8_t sigInit(void)
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_EUI64, 0);
	if (offset < 0xFFFF) {
		mStatus = SIG_GOOD;
		getEui64(mEUI64, offset);
	}
	else {
		// Init platform signature and check for board version.
		if (platform_eui(mEUI64)) {
			semver_t v = sigGetBoardVersion();
			if ((v.major > 0) || (v.minor > 0) || (v.patch > 0)) {
				mStatus = SIG_GOOD;
				return SIG_GOOD;
			}
		}

		uint16_t i;
		for (i=0;i<sigAreaGetSize();i++) { // Detect if uninitialized
			if (sigAreaReadByte(i) != 0xFF) {
				mStatus = SIG_BAD;
				return SIG_BAD;
			}
		}
		mStatus = SIG_EMPTY;
		return SIG_EMPTY;
	}
	return SIG_GOOD;
}

void sigGetEui64(uint8_t *buf)
{
	memcpy(buf, mEUI64, 8);
}

void sigSetEui64(uint8_t *buf)
{
	memcpy(mEUI64, buf, 8);
}

uint16_t sigGetNodeId(void)
{
	uint16_t addr = mEUI64[6] << 8 | mEUI64[7];
	if ((addr == 0)||(addr == 0xFFFF)) {
		addr = 1;
	}
	return addr;
}

semver_t sigGetSignatureVersion(void)
{
	semver_t v = {0,0,0};
	uint16_t offset = findSignature(0, 0);
	if (offset < 0xFFFF) {
		return getSignatureVersion(0);
	}
	return v;
}

semver_t sigGetBoardVersion(void)
{
	packed_semver_t pv = {0,0,0};
	semver_t v;

	uint16_t offset = findSignature(SIGNATURE_TYPE_BOARD, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, component_version_major), &pv, sizeof(pv));
	}
	else {
		offset = findSignature(0, 0);
		if (offset < 0xFFFF) {
			semver_t sigver = getSignatureVersion(offset);
			if (sigver.major == 1) {
				sigAreaRead(offset+offsetof(usersig_header_v1_t, pcb_version_major), &pv, sizeof(pv));
			}
			else if (sigver.major == 2) {
				sigAreaRead(offset+offsetof(usersig_header_v2_t, pcb_version_major), &pv, sizeof(pv));
			}
		}
	}

	v.major = pv.major;
	v.minor = pv.minor;
	v.patch = pv.patch;
	return v;
}

semver_t sigGetPlatformVersion(void)
{
	packed_semver_t pv = {0,0,0};
	semver_t v;

	uint16_t offset = findSignature(SIGNATURE_TYPE_PLATFORM, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, component_version_major), &pv, sizeof(pv));
	}
	else {
		return sigGetBoardVersion();
	}
	v.major = pv.major;
	v.minor = pv.minor;
	v.patch = pv.patch;
	return v;
}

int8_t sigGetBoardUUID(uint8_t uuid[16])
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_BOARD, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, component_uuid), uuid, USERSIG_UUID_LENGTH);
		return SIG_GOOD;
	}
	else { // version 1 or 2
		offset = findSignature(0, 0);
		if (offset < 0xFFFF) {
			semver_t sigver = getSignatureVersion(offset);
			if (sigver.major == 1) { // v1 does not have UUIDs
				memset(uuid, 0, USERSIG_UUID_LENGTH); // TODO implement the mapping
			}
			else if (sigver.major == 2) {
				sigAreaRead(offset+offsetof(usersig_header_v2_t, board_uuid), uuid, USERSIG_UUID_LENGTH);
				return SIG_GOOD;
			}
			else {
				memset(uuid, 0, USERSIG_UUID_LENGTH);
			}
		}
		else {
			memset(uuid, 0, USERSIG_UUID_LENGTH);
		}
	}
	return SIG_EMPTY;
}

int8_t sigGetPlatformUUID(uint8_t uuid[16])
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_PLATFORM, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, component_uuid), uuid, USERSIG_UUID_LENGTH);
	}
	else { // Fall back to board UUID for older versions or cases where there is no platform sig
		return sigGetBoardUUID(uuid);
	}
	return SIG_GOOD;
}

int8_t sigGetBoardManufacturerUUID(uint8_t uuid[16])
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_BOARD, 0);
	if (offset < 0xFFFF) {
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, manufacturer_uuid), uuid, USERSIG_UUID_LENGTH);
		return SIG_GOOD;
	}
	else { // version 1 or 2
		offset = findSignature(0, 0);
		if (offset < 0xFFFF) {
			semver_t sigver = getSignatureVersion(offset);
			if (sigver.major == 1) { // v1 does not have manufacturer info
				memset(uuid, 0, USERSIG_UUID_LENGTH);
			}
			else if (sigver.major == 2) {
				sigAreaRead(offset+offsetof(usersig_header_v2_t, manufacturer_uuid), uuid, USERSIG_UUID_LENGTH);
				return SIG_GOOD;
			}
			else {
				memset(uuid, 0, USERSIG_UUID_LENGTH);
			}
		}
		else {
			memset(uuid, 0, USERSIG_UUID_LENGTH);
		}
	}
	return SIG_EMPTY;
}

int8_t sigGetPlatformManufacturerUUID(uint8_t uuid[16])
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_PLATFORM, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, manufacturer_uuid), uuid, USERSIG_UUID_LENGTH);
	}
	else { // Fall back to board manufacturer if no platform manufacturer
		return sigGetBoardManufacturerUUID(uuid);
	}
	return SIG_GOOD;
}

void sigGetBoardName(char buf[], uint8_t length)
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_BOARD, 0);

	if(length < 16) {
		return; // The user must provide a buffer of 16 or 17 bytes, depending on the use-case
	}

	buf[0] = '\0'; // Make it an empty string, update if name found

	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, component_name), buf, 16);
	}
	else { // version 1 or 2
		offset = findSignature(0, 0);
		if (offset < 0xFFFF) {
			semver_t sigver = getSignatureVersion(offset);
			if (sigver.major == 1) {
				sigAreaRead(offset+offsetof(usersig_header_v1_t, board_name), buf, 16);
			}
			else if (sigver.major == 2) {
				sigAreaRead(offset+offsetof(usersig_header_v2_t, board_name), buf, 16);
			}
		}
	}
	if(length > 16) {
		buf[16] = '\0';
	}
}

void sigGetPlatformName(char buf[], uint8_t length)
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_PLATFORM, 0);

	if(length < 16) {
		return; // The user must provide a buffer of 16 or 17 bytes, depending on the use-case
	}

	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, component_name), buf, 16);
		if(length > 16) {
			buf[16] = '\0';
		}
	}
	else { // Fall back to board name for older versions or cases where there is no platform sig
		sigGetBoardName(buf, length);
	}
}

int64_t sigGetBoardProductionTime(void)
{
	int64_t timestamp = 0;
	uint16_t offset = findSignature(SIGNATURE_TYPE_BOARD, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, unix_time), &timestamp, sizeof(timestamp));
	}
	else {
		offset = findSignature(0, 0);
		if (offset < 0xFFFF) {
			semver_t sigver = getSignatureVersion(offset);
			if (sigver.major == 1) {
				sigAreaRead(offset+offsetof(usersig_header_v1_t, unix_time), &timestamp, sizeof(timestamp));
			}
			else if (sigver.major == 2) {
				sigAreaRead(offset+offsetof(usersig_header_v2_t, unix_time), &timestamp, sizeof(timestamp));
			}
		}
	}
	timestamp = ntoh64(timestamp);
	return timestamp;
}

int64_t sigGetPlatformProductionTime(void)
{
	int64_t timestamp = 0;
	uint16_t offset = findSignature(SIGNATURE_TYPE_PLATFORM, 0);
	if (offset < 0xFFFF) { // This is version 3 (or later)
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, unix_time), &timestamp, sizeof(timestamp));
		timestamp = ntoh64(timestamp);
	}
	else { // version 1 or 2
		return sigGetBoardProductionTime();
	}
	return timestamp;
}

void sigGetBoardSerial(uint8_t serial[16])
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_BOARD, 0);
	if (offset < 0xFFFF) {
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, serial_number), serial, 16);
	}
	else {
		memset(serial, 0, 16);
	}
}

void sigGetPlatformSerial(uint8_t serial[16])
{
	uint16_t offset = findSignature(SIGNATURE_TYPE_BOARD, 0);
	if (offset < 0xFFFF) {
		sigAreaRead(offset+offsetof(usersig_header_v3_component_t, serial_number), serial, 16);
	}
	else {
		sigGetBoardSerial(serial);
	}
}

uint16_t sigGetLicenseFile(uint8_t buf[])
{
	uint16_t sz = 0;
	uint16_t offset = findSignature(SIGNATURE_TYPE_LICENSE, 0);
	if (offset < 0xFFFF)
	{
		sigAreaRead(offset+sizeof(packed_semver_t), &sz, sizeof(sz));
		sz = ntoh16(sz) - sizeof(usersig_header_v3_t) - 2; // Size - header - crc
		sigAreaRead(offset+sizeof(usersig_header_v3_t), buf, sz);
		return sz;
	}
	return sz;
}

uint16_t sigGetLength(void)
{
	uint16_t out;
	uint16_t signlen = findSignature(SIGNATURE_TYPE_BOARD, 0);

	while (sigAreaGetSize() > signlen)
	{
		out = (sigAreaReadByte(signlen+3) << 8) | sigAreaReadByte(signlen+4);

		if (out == 0xFFFF)
		{
			break;
		}
		signlen = signlen + out;
	}

	if (signlen > sigAreaGetSize())
	{
		return 0;
	}
	return signlen;
}

// -----------------------------------------------------------------------------
// Element API
// -----------------------------------------------------------------------------

uint16_t sigFirstElement(uint8_t tp)
{
	return findSignature(tp, 0);
}

uint16_t sigNextElement(uint8_t tp, uint16_t offset)
{
	uint16_t length = getSignatureLength(offset);
	return findSignature(tp, offset+length);
}

int8_t sigGetElementVersion(uint8_t tp, semver_t* v, uint16_t sig_offset)
{
	packed_semver_t pv;
	uint16_t o = findSignature(tp, sig_offset);
	if (o == sig_offset) {
		sigAreaRead(sig_offset+offsetof(usersig_header_v3_component_t, component_version_major), &pv, sizeof(pv));
		v->major = pv.major;
		v->minor = pv.minor;
		v->patch = pv.patch;
		return 0;
	}
	return -1;
}

int8_t sigGetElementProductionTime(uint8_t tp, int64_t* timestamp, uint16_t sig_offset)
{
	int64_t ts = 0;
	uint16_t o = findSignature(tp, sig_offset);
	if (o == sig_offset) {
		sigAreaRead(sig_offset+offsetof(usersig_header_v3_component_t, unix_time), &ts, sizeof(ts));
		*timestamp = ntoh64(ts);
		return 0;
	}
	return -1;
}

int8_t sigGetElementManufacturerUUID(uint8_t tp, uint8_t uuid[16], uint16_t sig_offset)
{
	uint16_t o = findSignature(tp, sig_offset);
	if (o == sig_offset) {
		sigAreaRead(sig_offset+offsetof(usersig_header_v3_component_t, manufacturer_uuid), uuid, USERSIG_UUID_LENGTH);
		return 0;
	}
	return -1;
}

int8_t sigGetElementUUID(uint8_t tp, uint8_t uuid[16], uint16_t sig_offset)
{
	uint16_t o = findSignature(tp, sig_offset);
	if (o == sig_offset) {
		sigAreaRead(sig_offset+offsetof(usersig_header_v3_component_t, component_uuid), uuid, USERSIG_UUID_LENGTH);
		return 0;
	}
	return -1;
}

int8_t sigGetElementName(uint8_t tp, char name[17], uint16_t sig_offset)
{
	uint16_t o = findSignature(tp, sig_offset);
	if (o == sig_offset) {
		sigAreaRead(sig_offset+offsetof(usersig_header_v3_component_t, component_name), name, 16);
		name[16] = '\0';
		return 0;
	}
	return -1;
}

int8_t sigGetElementSerial(uint8_t tp, uint8_t serial[16], uint16_t sig_offset)
{
	uint16_t o = findSignature(tp, sig_offset);
	if (o == sig_offset) {
		sigAreaRead(sig_offset+offsetof(usersig_header_v3_component_t, serial_number), serial, 16);
		return 0;
	}
	return -1;
}

int8_t sigGetElementPosition(uint8_t tp, uint8_t* position, uint16_t sig_offset)
{
	uint16_t o = findSignature(tp, sig_offset);
	if (o == sig_offset) {
		sigAreaRead(sig_offset+offsetof(usersig_header_v3_component_t, position), position, sizeof(uint8_t));
		return 0;
	}
	return -1;
}

int32_t sigGetElementDataLength(uint8_t tp, uint16_t sig_offset)
{
	uint16_t len;
	uint16_t o = findSignature(tp, sig_offset);
	if (o == sig_offset) {
		sigAreaRead(sig_offset+offsetof(usersig_header_v3_component_t, data_length), &len, sizeof(len));
		return ntoh16(len);
	}
	return -1;
}

int32_t sigGetElementData(uint8_t tp, uint8_t* buffer, uint16_t length, uint16_t data_offset, uint8_t sig_offset)
{
	uint16_t o = findSignature(tp, sig_offset);
	if (o == sig_offset) {
		uint16_t len;
		sigAreaRead(sig_offset+offsetof(usersig_header_v3_component_t, data_length), &len, sizeof(len));
		len = ntoh16(len);
		if (len <= data_offset) {
			return 0;
		}
		if (data_offset + length > len) {
			length = len - data_offset;
		}
		sigAreaRead(sig_offset+offsetof(usersig_header_v3_component_t, data_length)+data_offset, buffer, length);
		return length;
	}
	return -1;
}

// -----------------------------------------------------------------------------
// Component API
// -----------------------------------------------------------------------------

uint16_t sigFirstComponent()
{
	return sigFirstElement(SIGNATURE_TYPE_COMPONENT);
}
uint16_t sigNextComponent(uint16_t offset)
{
	return sigNextElement(SIGNATURE_TYPE_COMPONENT, offset);
}
int8_t sigGetComponentVersion(semver_t* v, uint16_t sig_offset)
{
	return sigGetElementVersion(SIGNATURE_TYPE_COMPONENT, v, sig_offset);
}
int8_t sigGetComponentProductionTime(int64_t* timestamp, uint16_t sig_offset)
{
	return sigGetElementProductionTime(SIGNATURE_TYPE_COMPONENT, timestamp, sig_offset);
}
int8_t sigGetComponentManufacturerUUID(uint8_t uuid[16], uint16_t sig_offset)
{
	return sigGetElementManufacturerUUID(SIGNATURE_TYPE_COMPONENT, uuid, sig_offset);
}
int8_t sigGetComponentUUID(uint8_t uuid[16], uint16_t sig_offset)
{
	return sigGetElementUUID(SIGNATURE_TYPE_COMPONENT, uuid, sig_offset);
}
int8_t sigGetComponentName(char name[17], uint16_t sig_offset)
{
	return sigGetElementName(SIGNATURE_TYPE_COMPONENT, name, sig_offset);
}
int8_t sigGetComponentSerial(uint8_t serial[16], uint16_t sig_offset)
{
	return sigGetElementSerial(SIGNATURE_TYPE_COMPONENT, serial, sig_offset);
}
int8_t sigGetComponentPosition(uint8_t* position, uint16_t sig_offset)
{
	return sigGetElementPosition(SIGNATURE_TYPE_COMPONENT, position, sig_offset);
}
int32_t sigGetComponentDataLength(uint16_t sig_offset)
{
	return sigGetElementDataLength(SIGNATURE_TYPE_COMPONENT, sig_offset);
}
int32_t sigGetComponentData(uint8_t* buffer, uint16_t length, uint16_t data_offset, uint8_t sig_offset)
{
	return sigGetElementData(SIGNATURE_TYPE_COMPONENT, buffer, length, data_offset, sig_offset);
}

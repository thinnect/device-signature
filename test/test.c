#include <stdio.h>
#include <inttypes.h>

#include "SignatureArea.h"
#include "SignatureAreaFile.h"

#include "DeviceSignature.h"

void sprintEui64(char seui[17], const uint8_t eui[8]) {
	snprintf(seui, 17, "%02X%02X%02X%02X%02X%02X%02X%02X", eui[0], eui[1], eui[2], eui[3], eui[4], eui[5], eui[6], eui[7]);
}

void sprintUUID(char suuid[37], const uint8_t uuid[16]) {
	snprintf(suuid, 37,
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
	    uuid[0], uuid[1],  uuid[2],  uuid[3],  uuid[4],  uuid[5],  uuid[6],  uuid[7],
	    uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
}

void printSignatureInfo() {
	uint8_t eui[8];
	char seui[17];
	uint8_t uuid[16];
	char suuid[37];
	char name[17];
	semver_t ver;
	int64_t ts;
	uint8_t serial[17];
	char sserial[37];

	sigGetEui64(eui);
	sprintEui64(seui, eui);
	printf("        EUI: %s\n", seui);

	sigGetBoardUUID(uuid);
	sprintUUID(suuid, uuid);
	printf(" Board UUID: %s\n", suuid);

	sigGetBoardName(name, sizeof(name));
	printf("      Board: %s\n", name);

	ver = sigGetBoardVersion();
	printf("    Version: %d.%d.%d\n", ver.major, ver.minor, ver.patch);

	sigGetBoardSerial(serial);
	serial[16] = '\0';
	sprintUUID(sserial, serial);
	printf("     Serial: %s (%s)\n", serial, sserial);

	sigGetBoardManufacturerUUID(uuid);
	sprintUUID(suuid, uuid);
	printf("  Mfgr UUID: %s\n", suuid);

	ts = sigGetBoardProductionTime();
	printf("  Prod time: %"PRIi64"\n", ts);

	sigGetPlatformUUID(uuid);
	sprintUUID(suuid, uuid);
	printf("Pltfrm UUID: %s\n", suuid);

	sigGetPlatformName(name, sizeof(name));
	printf("   Platform: %s\n", name);

	ver = sigGetPlatformVersion();
	printf("    Version: %d.%d.%d\n", ver.major, ver.minor, ver.patch);

	sigGetPlatformSerial(serial);
	serial[16] = '\0';
	sprintUUID(sserial, serial);
	printf("     Serial: %s (%s)\n", serial, sserial);

	sigGetPlatformManufacturerUUID(uuid);
	sprintUUID(suuid, uuid);
	printf("  Mfgr UUID: %s\n", suuid);

	ts = sigGetPlatformProductionTime();
	printf("  Prod time: %"PRIi64"\n", ts);
}

void printComponentInfo(uint16_t offset) {
	uint8_t uuid[16];
	char suuid[37];
	char name[17];
	semver_t ver;
	int64_t ts;
	uint8_t position;
	uint8_t serial[17];
	char sserial[37];
	int32_t dataLength;

	sigGetComponentUUID(uuid, offset);
	sprintUUID(suuid, uuid);
	printf("   Cmp UUID: %s\n", suuid);

	sigGetComponentName(name, offset);
	printf("       Name: %s\n", name);

	sigGetComponentVersion(&ver, offset);
	printf("    Version: %d.%d.%d\n", ver.major, ver.minor, ver.patch);

	sigGetComponentManufacturerUUID(uuid, offset);
	sprintUUID(suuid, uuid);
	printf("  Mfgr UUID: %s\n", suuid);

	sigGetComponentProductionTime(&ts, offset);
	printf("  Prod time: %"PRIi64"\n", ts);

	sigGetComponentPosition(&position, offset);
	printf("   Position: %"PRIu8"\n", position);

	sigGetComponentSerial(serial, offset);
	serial[16] = '\0';
	sprintUUID(sserial, serial);
	printf("     Serial: %s (%s)\n", serial, sserial);

	dataLength = sigGetComponentDataLength(offset);
	printf("       Data: %"PRIi32"\n", dataLength);
}

int main() {
	sigAreaInit("sig-v1.bin");
	printf("size=%d\n", sigAreaGetSize());
	printf("init=%d\n", sigInit());
	printf("\n");
	printSignatureInfo();
	printf("\n");

	sigAreaInit("sig-v2.bin");
	printf("size=%d\n", sigAreaGetSize());
	printf("init=%d\n", sigInit());
	printf("\n");
	printSignatureInfo();
	printf("\n");

	sigAreaInit("sig-v3-board.bin");
	printf("size=%d\n", sigAreaGetSize());
	printf("init=%d\n", sigInit());
	printf("\n");
	printSignatureInfo();
	printf("\n");

	sigAreaInit("sig-v3-platform.bin");
	printf("size=%d\n", sigAreaGetSize());
	printf("init=%d\n", sigInit());
	printf("\n");
	printSignatureInfo();
	printf("\n");

	sigAreaInit("EUI-64_DEADBEEF1234567F.bin");
	printf("size=%d\n", sigAreaGetSize());
	printf("init=%d\n", sigInit());
	printf("\n");
	printSignatureInfo();
	printf("\n");

	uint16_t offset = sigFirstComponent();
	while(offset < 0xFFFF) {
		printf("sig @ 0x%04X\n", offset);
		printComponentInfo(offset);
		printf("\n");
		offset = sigNextComponent(offset);
	}

	printf("done\n");
}

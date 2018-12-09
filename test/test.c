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
	uint8_t suuid[37];
	char name[17];
	semver_t pcbv;
	int64_t ts;

	sigGetEui64(eui);
	sprintEui64(seui, eui);
	printf("        EUI: %s\n", seui);

	sigGetPlatformUUID(uuid);
	sprintUUID(suuid, uuid);
	printf("Pltfrm UUID: %s\n", suuid);

	sigGetBoardUUID(uuid);
	sprintUUID(suuid, uuid);
	printf(" Board UUID: %s\n", suuid);

	sigGetManufacturerUUID(uuid);
	sprintUUID(suuid, uuid);
	printf("  Mfgr UUID: %s\n", suuid);

	sigGetBoardName(name);
	printf("      Board: %s\n", name);

	sigGetPlatformName(name);
	printf("   Platform: %s\n", name);

	pcbv = sigGetPcbVersion();
	printf("        PCB: %d.%d.%d\n", pcbv.major, pcbv.minor, pcbv.patch);

	ts = sigGetProductionTime();
	printf("  Prod time: %"PRIi64"\n", ts);
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

	printf("done\n");
}

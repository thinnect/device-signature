/**
 * Device signature API.
 *
 * @author Konstantin Bilozor, Raido Pahtma
 * @license MIT
 */
#ifndef USERSIGNATUREAREA_H_
#define USERSIGNATUREAREA_H_

enum UserSignatureAreaEnum {
//	USERSIG_VERSION_MAJOR = 1,
//	USERSIG_VERSION_MINOR = 0,
//	USERSIG_SIZE = 768,

	USERSIG_BOARDNAME_MAX_STRLEN = 16,
	USERSIG_UUID_LENGTH = 16,
};

enum SigInitResults {
	SIG_OFF   = -128,
	SIG_GOOD  = 0,
	SIG_BAD   = 1,
	SIG_EMPTY = 127
};

typedef struct semver {
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
} semver_t;

#pragma pack(push, 1)
typedef struct packed_semver {
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
} packed_semver_t;
#pragma pack(pop)

enum SignatureTypes {
	SIGNATURE_TYPE_EUI64 = 0,    // EUI64 is the IEEE Extended Unique Identifier
	SIGNATURE_TYPE_BOARD = 1,    // Boards are the core of the system - MCU
	SIGNATURE_TYPE_PLATFORM = 2, // Platforms define the set of components
	SIGNATURE_TYPE_COMPONENT = 3 // Components list individual parts of a platform
};

int8_t   sigInit(void);
void     sigGetEui64(uint8_t *buf);
semver_t sigGetPcbVersion(void);
void     sigGetBoardUUID(uint8_t uuid[16]);
void     sigGetPlatformUUID(uint8_t uuid[16]);
void     sigGetManufacturerUUID(uint8_t uuid[16]);
void     sigGetBoardName(uint8_t buf[16]);
void     sigGetPlatformName(uint8_t buf[16]);
int64_t  sigGetProductionTime(void);

semver_t sigGetSignatureVersion(void);

uint16_t sigGetNodeId(void);

#endif // USERSIGNATUREAREA_H_

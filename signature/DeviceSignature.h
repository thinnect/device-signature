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

semver_t sigGetBoardVersion(void);
semver_t sigGetPlatformVersion(void);
void     sigGetBoardUUID(uint8_t uuid[16]);
void     sigGetPlatformUUID(uint8_t uuid[16]);
void     sigGetBoardManufacturerUUID(uint8_t uuid[16]);
void     sigGetPlatformManufacturerUUID(uint8_t uuid[16]);
void     sigGetBoardName(uint8_t buf[16]);
void     sigGetPlatformName(uint8_t buf[16]);
int64_t  sigGetBoardProductionTime(void);
int64_t  sigGetPlatformProductionTime(void);
void     sigGetBoardSerial(uint8_t serial[16]);
void     sigGetPlatformSerial(uint8_t serial[16]);

semver_t sigGetSignatureVersion(void);

uint16_t sigGetNodeId(void);


// -----------------------------------------------------------------------------
// Component API, use sigFindComponent to search for component info
// and retrieve details using the returned offset. Commands return positive
// values on SUCCESS.
// -----------------------------------------------------------------------------

// Start searching for a component signature from the specified offset.
// returns the offset of the found component entry or 0xFFFF if nothing found.
uint16_t sigFirstComponent();

// Find the next component, offset must belong to a previous signature entry.
// returns the offset of the found component entry or 0xFFFF if nothing found.
uint16_t sigNextComponent(uint16_t offset);

int8_t sigGetComponentUUID(uint8_t uuid[16], uint16_t offset);
int8_t sigGetComponentName(uint8_t name[17], uint16_t offset);
int8_t sigGetComponentVersion(semver_t* v, uint16_t offset);
int8_t sigGetComponentProductionTime(int64_t* timestamp, uint16_t offset);
int8_t sigGetComponentManufacturerUUID(uint8_t uuid[16], uint16_t offset);
int8_t sigGetComponentSerial(uint8_t serial[16], uint16_t offset);
int8_t sigGetComponentPosition(uint8_t* position, uint16_t offset);

int32_t sigGetComponentDataLength(uint16_t offset);
int32_t sigGetComponentData(uint8_t* buffer, uint16_t bufferLength, uint16_t dataOffset, uint8_t offset);

// -----------------------------------------------------------------------------
// Low-level element API
// Works the same as the component API.
// -----------------------------------------------------------------------------
uint16_t sigFirstElement(uint8_t tp);
uint16_t sigNextElement(uint8_t tp, uint16_t offset);

int8_t sigGetElementUUID(uint8_t tp, uint8_t uuid[16], uint16_t offset);
int8_t sigGetElementName(uint8_t tp, uint8_t name[17], uint16_t offset);
int8_t sigGetElementVersion(uint8_t tp, semver_t* v, uint16_t offset);
int8_t sigGetElementProductionTime(uint8_t tp, int64_t* timestamp, uint16_t offset);
int8_t sigGetElementManufacturerUUID(uint8_t tp, uint8_t uuid[16], uint16_t offset);
int8_t sigGetElementSerial(uint8_t tp, uint8_t serial[16], uint16_t offset);
int8_t sigGetElementPosition(uint8_t tp, uint8_t* position, uint16_t offset);

int32_t sigGetElementDataLength(uint8_t tp, uint16_t offset);
int32_t sigGetElementData(uint8_t tp, uint8_t* buffer, uint16_t bufferLength, uint16_t dataOffset, uint8_t offset);

#endif // USERSIGNATUREAREA_H_

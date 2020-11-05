/**
 * Device signature API.
 *
 * @author Konstantin Bilozor, Raido Pahtma
 * @license MIT
 */
#ifndef DEVICESIGNATURE_H_
#define DEVICESIGNATURE_H_

enum UserSignatureAreaEnum {
	USERSIG_BOARDNAME_MAX_STRLEN = 16,
	USERSIG_UUID_LENGTH = 16,
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
	SIGNATURE_TYPE_EUI64 = 0,     // EUI64 is the IEEE Extended Unique Identifier
	SIGNATURE_TYPE_BOARD = 1,     // Boards are the core of the system - MCU
	SIGNATURE_TYPE_PLATFORM = 2,  // Platforms define the set of components
	SIGNATURE_TYPE_COMPONENT = 3, // Components list individual parts of a platform
	SIGNATURE_TYPE_LICENSE = 4    // License file
};

enum SigInitResults {
	SIG_OFF   = -128,
	SIG_GOOD  = 0,
	SIG_BAD   = 1,
	SIG_EMPTY = 127
};

// -----------------------------------------------------------------------------
// General signature - board & platform API
// -----------------------------------------------------------------------------

/**
 * Initialize the signature module, load EUI64.
 * EUI gets initialized to all 0 on errors and all ff when no signature found.
 *
 * @return One of SigInitResults.
 */
int8_t   sigInit(void);

/**
 * Copy the EUI64 into the specified buffer
 *
 * @param buf A 16 byte buffer.
 */
void     sigGetEui64(uint8_t * buf);

/**
 * Override the EUI64, not persistent
 * @param buf 8-byte EUI64 buffer.
 */
void     sigSetEui64(uint8_t buf[8]);

/**
 * Returns the last 2 bytes of the EUI64, takes care not to return 0 or bcast.
 *
 * @return The last 2 bytes of the EUI64 or 1 when no EUI64.
 */
uint16_t sigGetNodeId(void);

semver_t sigGetBoardVersion(void);
semver_t sigGetPlatformVersion(void);
int8_t   sigGetBoardUUID(uint8_t uuid[16]);
int8_t   sigGetPlatformUUID(uint8_t uuid[16]);
int8_t   sigGetBoardManufacturerUUID(uint8_t uuid[16]);
int8_t   sigGetPlatformManufacturerUUID(uint8_t uuid[16]);

/**
 * Get the board name, 0 padded and 16 characters long. Will be 0 terminated
 * if 17 bytes, may not be, if 16.
 *
 * @param buf A buffer to store the name, at least 16 bytes.
 * @param length The buffer length, guaranteed to be 0 terminated if > 16.
 */
void     sigGetBoardName(char buf[], uint8_t length);

/**
 * Get the platform name, 0 padded and 16 characters long. Will be 0 terminated
 * if 17 bytes, may not be, if 16.
 *
 * @param buf A buffer to store the name, at least 16 bytes.
 * @param length The buffer length, guaranteed to be 0 terminated if > 16.
 */
void     sigGetPlatformName(char buf[], uint8_t length);

/**
 * Get license file.
 * @param buf A buffer to store license file.
 *
 * @return Returns license file length.
 */
uint16_t sigGetLicenseFile(uint8_t buf[]);

/**
 * Get length of the signature.
 *
 * @return Returns signature length. 0xFFFF if fails.
 */
uint16_t sigGetLength(void);

int64_t  sigGetBoardProductionTime(void);
int64_t  sigGetPlatformProductionTime(void);
void     sigGetBoardSerial(uint8_t serial[16]);
void     sigGetPlatformSerial(uint8_t serial[16]);

semver_t sigGetSignatureVersion(void);

// -----------------------------------------------------------------------------
// Component API, use sigFindComponent to search for component info
// and retrieve details using the returned offset. Commands return positive
// values on SUCCESS.
// -----------------------------------------------------------------------------

/**
 * Find the first signature component.
 *
 * @return the offset of the found component entry or 0xFFFF if nothing found.
 */
uint16_t sigFirstComponent();

/**
 * Find the next component, offset must belong to a previous signature entry.
 *
 * @return The offset of the found component entry or 0xFFFF if nothing found.
 */
uint16_t sigNextComponent(uint16_t offset);

int8_t sigGetComponentUUID(uint8_t uuid[16], uint16_t offset);
int8_t sigGetComponentName(char name[17], uint16_t offset);
int8_t sigGetComponentVersion(semver_t* v, uint16_t offset);
int8_t sigGetComponentProductionTime(int64_t* timestamp, uint16_t offset);
int8_t sigGetComponentManufacturerUUID(uint8_t uuid[16], uint16_t offset);
int8_t sigGetComponentSerial(uint8_t serial[16], uint16_t offset);
int8_t sigGetComponentPosition(uint8_t* position, uint16_t offset);

int32_t sigGetComponentDataLength(uint16_t offset);
int32_t sigGetComponentData(uint8_t* buf, uint16_t bufLen, uint16_t dataOffset, uint8_t offset);



// -----------------------------------------------------------------------------
// Low-level element API
// Works the same as the component API.
// -----------------------------------------------------------------------------
uint16_t sigFirstElement(uint8_t tp);
uint16_t sigNextElement(uint8_t tp, uint16_t offset);

int8_t sigGetElementUUID(uint8_t tp, uint8_t uuid[16], uint16_t offset);
int8_t sigGetElementName(uint8_t tp, char name[17], uint16_t offset);
int8_t sigGetElementVersion(uint8_t tp, semver_t* v, uint16_t offset);
int8_t sigGetElementProductionTime(uint8_t tp, int64_t* timestamp, uint16_t offset);
int8_t sigGetElementManufacturerUUID(uint8_t tp, uint8_t uuid[16], uint16_t offset);
int8_t sigGetElementSerial(uint8_t tp, uint8_t serial[16], uint16_t offset);
int8_t sigGetElementPosition(uint8_t tp, uint8_t* position, uint16_t offset);

int32_t sigGetElementDataLength(uint8_t tp, uint16_t offset);
int32_t sigGetElementData(uint8_t tp, uint8_t* buf, uint16_t bufLen, uint16_t dataOffset, uint8_t offset);

#endif // DEVICESIGNATURE_H_

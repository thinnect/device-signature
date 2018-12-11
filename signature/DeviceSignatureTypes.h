/**
 * Device signature structures.
 *
 * @author Konstantin Bilozor, Raido Pahtma
 * @license MIT
 */
#ifndef USERSIGNATUREAREATYPES_H_
#define USERSIGNATUREAREATYPES_H_

#include "DeviceSignature.h"

#pragma pack(push, 1)
typedef struct usersig_header_v1 {
	uint8_t version_major;
	uint8_t version_minor;
	uint8_t version_patch;

	uint8_t eui64[8];

	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;

	int64_t unix_time; // Same moment in time

	uint8_t board_name[USERSIG_BOARDNAME_MAX_STRLEN]; // up to 16 chars or 0 terminated

	uint8_t pcb_version_major;
	uint8_t pcb_version_minor;
	uint8_t pcb_version_assembly;

	uint8_t board_uuid[USERSIG_UUID_LENGTH];
	uint8_t manufacturer_uuid[USERSIG_UUID_LENGTH];

	uint8_t padding[123];
} usersig_header_v1_t;

typedef struct usersig_header_v2 {
	uint8_t version_major;
	uint8_t version_minor;
	uint8_t version_patch;

	uint16_t signature_size;
	uint8_t eui64[8];

	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;

	int64_t unix_time; // Same moment in time

	uint8_t board_name[USERSIG_BOARDNAME_MAX_STRLEN]; // up to 16 chars or 0 terminated

	uint8_t pcb_version_major;
	uint8_t pcb_version_minor;
	uint8_t pcb_version_assembly;

	uint8_t board_uuid[USERSIG_UUID_LENGTH];
	uint8_t manufacturer_uuid[USERSIG_UUID_LENGTH];
} usersig_header_v2_t;

typedef struct usersig_header_v3_eui {
	uint8_t version_major;
	uint8_t version_minor;
	uint8_t version_patch;

	uint16_t signature_size;

	uint8_t signature_type; // 0

	int64_t unix_time; // Same moment in time

	uint8_t eui64[8];
} usersig_header_v3_eui_t;

typedef struct usersig_header_v3_component {
	uint8_t version_major;
	uint8_t version_minor;
	uint8_t version_patch;

	uint16_t signature_size;

	uint8_t signature_type; // 2

	int64_t unix_time; // Manufacturing timestamp

	uint8_t component_uuid[USERSIG_UUID_LENGTH];
	uint8_t component_name[USERSIG_BOARDNAME_MAX_STRLEN]; // 16 chars or 0 terminated

	uint8_t component_version_major;
	uint8_t component_version_minor;
	uint8_t component_version_assembly;

	uint8_t serial_number[USERSIG_UUID_LENGTH];

	uint8_t manufacturer_uuid[USERSIG_UUID_LENGTH];

	uint8_t position;

	uint16_t data_length; // Length of the following data array
	uint8_t data[];      // Component specific data - calibration for example
} usersig_header_v3_component_t;
#pragma pack(pop)

#endif // USERSIGNATUREAREATYPES_H_

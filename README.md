# device-signature
API and modules for accessing device signature data. The signature data is
usually a separate page in the MCU memory that is written once (or in steps)
during manufacturing and is later used by firmware to determine what hardware
it is running on. One of the main functions is to store the devices EUI64
identifier.

Signatures can be generated with the
[usersiggen](https://github.com/thinnect/euisiggen) tool.

## Supported signature versions

By default the library can interpret signature versions 1, 2 and 3, using board
properties for platform elements when encountering versions 1 and 2.

### Version 1
Used with most Atmel RFR2 based controllers, like the MURP module.

### Version 2
Temporarily used during an initial prototype run of some SiLabs based modules.

### Version 3
Used with some newer Atmel RFR2 based controllers and all SiLabs Gecko based
boards in production.

## Signature elements

### EUI64
The EUI64 element stores the devices identifier and a timestamp.

### Board
The board element stores the board name, UUID, version, serial-number,
manufacturer UUID and timestamp.

### Platform
The platform element stores the platform name, UUID, version, serial-number,
manufacturer UUID and timestamp.

### Generic component
The component element stores the component name, UUID, version, serial-number,
manufacturer UUID, timestamp and component position. There may be several
component elements in a signature.

#### Serial numbers
The current signature version does not specify what format the serial number
field in the elements should take, but the recommended approach is to first test
if the serial number is an UUID, then see if it is printable (ASCII) and
finally, if everything else fails, then to just encode it as HEX.

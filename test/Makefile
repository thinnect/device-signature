
INCLUDES = -I. -I../area -I../area/file -I../signature
INCLUDES += -Izoo/jtbr.endianness
INCLUDES += -Izoo/lammertb.libcrc/include

SOURCES = test.c ../area/file/SignatureArea.c ../signature/DeviceSignature.c
SOURCES += zoo/lammertb.libcrc/src/crcccitt.c

all:
	gcc -std=c99 -Wall $(INCLUDES) $(SOURCES) -o test-app

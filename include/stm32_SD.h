
#ifndef stm32_SD_h
#define stm32_SD_h
/*
SD card library
Based on :
https://github.com/dilshan/sdfatlib
*/

extern "C" {
//https://youtu.be/KM6rtL_bOto
#include "../openfat/include/openfat.h"
#include "../openfat/include/openfat/mbr.h"
}
#include <libopencm3/stm32/spi.h>
#include <stdint.h>
#include <fcntl.h>
#include "digio.h"
#include "sdcmd.h"
#include "printf.h"
#include "params.h"


// SD card types.
#define CT_MMC		0x01            // MMC version 3.
#define CT_SD1		0x02            // SD version 1.
#define CT_SD2		0x04            // SD version 2.
#define CT_SDC		(CT_SD1|CT_SD2) // SD.
#define CT_BLOCK	0x08            // Block addressing.

#define TOKEN_SINGLE_READ   0xFE
#define TOKEN_STOP_TRAN     0xFD
#define TOKEN_SINGLE_WRITE	0xFE
#define TOKEN_MULTI_WRITE	0xFC



class stm32_SD
{

public:
static int8_t sdDrvInit();
static int8_t OpenFatInit();
static void SDTest1();
static void CreateDir();
static void CreateFile();
static void WriteToFile(char *arr, uint8_t arr_S);

private:

};

static uint8_t sdDrvCommand(uint8_t cmd, uint32_t arg);
static int sdDrvGetBlock(uint8_t *buf, int len);
static int sdDrvSetBlock(const uint8_t *buf, int len, uint8_t token);

int sdDrvReadSector(const struct block_device *bldev, uint32_t sector, uint32_t count, void *buf);
int sdDrvWriteSector(const struct block_device *bldev, uint32_t sector, uint32_t count, const void *buf);
uint16_t sdDrvGetSectorSize(const struct block_device *dev);
#endif /* stm32_SD_h */

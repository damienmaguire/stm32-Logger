/******************************************************************************
 * SD FAT library for STM32 ARM Cortex-M3 MCUs.
 * Copyright (C) 2021 Dilshan R Jayakody [jayakody2000lk@gmail.com]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *****************************************************************************
 * This library is based on OpenFAT project written by
 * Gareth McMullin <gareth@blacksphere.co.nz>.
 * <https://github.com/tmolteno/openfat>
 *****************************************************************************/

#include <stdint.h>
#include <stdio.h>

#include "sdcmd.h"
#include "sdspi.h"
#include "sddriver.h"
#include "logger.h"

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

uint8_t cardType;

static uint8_t sdDrvCommand(uint8_t cmd, uint32_t arg)
{
    uint8_t buf[6];
    uint8_t ret;

    // Start + command index.
    buf[0] = cmd | 0x40;

    // ACMDn is the command sequence of CMD55-CMDn.
    if (buf[0] & 0x80)
    {
        buf[0] &= 0x7F;
        ret = sdDrvCommand(SD_CMD55, 0);
        if (ret > 1)
        {
            return ret;
        }
    }

    // Argument data [31...24].
    buf[1] = (arg >> 24) & 0xFF;

    // Argument data [23...16].
    buf[2] = (arg >> 16) & 0xFF;

    // Argument data [15...8].
    buf[3] = (arg >> 8) & 0xFF;

    // Argument data [7...0].
    buf[4] = arg & 0xFF;

    // CRC byte.
    if(cmd == SD_CMD0)
    {
        // Pre calculated CRC for idle state.
        buf[5] = 0x95;
    }
    else if(cmd == SD_CMD8)
    {
        // Pre calculated CRC for CMD8 (with arg data 0x1AA).
        buf[5] = 0x87;
    }
    else
    {
        // Dummy CRC and stop.
        buf[5] = 0x01;
    }

    sdSPISelect();
    sdSPIWriteBuffer(buf, sizeof(buf));

    do
    {
        // Wait for response byte.
        ret = sdSPIreadwrite(0xFF);
    }
    while(ret & 0x80);

    return ret;
}

static int sdDrvGetBlock(uint8_t *buf, int len)
{
    // Wait until SD card to getting into idle.
    loggerOutputLine("sdDrvGetBlock");
    while((*buf = sdSPIreadwrite(0xFF)) == 0xFF);

    if(*buf != TOKEN_SINGLE_READ)
    {
        // Invalid token.
        loggerOutputLine("sdDrvGetBlock::Token error");
        return -1;
    }

    sdSPIReadBuffer(buf, len);

    // Ignore CRC word.
    loggerOutputLine("sdDrvGetBlock::CRC");
    sdSPIreadwrite(0xFF);
    sdSPIreadwrite(0xFF);

    return 0;
}

static int sdDrvSetBlock(const uint8_t *buf, int len, uint8_t token)
{
    // Wait until SD card to getting into idle.
    loggerOutputLine("sdDrvSetBlock");
    while(sdSPIreadwrite(0xFF) != 0xFF);

    // Send token.
    loggerOutputLine("sdDrvSetBlock::Token");
    sdSPIreadwrite(token);

    if (token != TOKEN_STOP_TRAN)
    {
        // Data token, and send data buffer.
        loggerOutputLine("sdDrvSetBlock::Data Token");
        sdSPIWriteBuffer(buf, len);

        // Send dummy CRC word.
        loggerOutputLine("sdDrvSetBlock::CRC");
        sdSPIreadwrite(0xFF);
        sdSPIreadwrite(0xFF);

        // Response status of the transfer operation.
        loggerOutputLine("sdDrvSetBlock::Status");
        return sdSPIreadwrite(0xFF);
    }

    return 0;
}

int8_t sdDrvInit()
{
    uint8_t ocr[4];
    uint8_t temp, cmd;

    cardType = 0;

    loggerOutputLine("sdDrvInit");
    loggerOutputLine("sdDrvInit::sdCardIntfInit");
    sdCardIntfInit();

    // Start SD card initialization with dummy write.
    loggerOutputLine("sdDrvInit::Dummy write");
    for(temp = 0 ; temp < 80; temp++)
    {
        sdSPIreadwrite(0xFF);
    }

    loggerOutputLine("sdDrvInit::SD_CMD0");
    if(sdDrvCommand(SD_CMD0, 0) != 1)
	{
            // Unable to reset the SD card.
            loggerOutputLine("sdDrvInit::SD_CMD0 fail");
            return -1;
	}

    // Identify the Version 1.x or Version 2.x SD card.
    if(sdDrvCommand(SD_CMD8, 0x1AA) == 1)
    {
        // SDHC is detected, try to get R7 response.
        loggerOutputLine("sdDrvInit::SDHC");
        sdSPIReadBuffer(ocr, sizeof(ocr));

        // Check card is support for 2.7V - 3.6V.
        if((ocr[2] == 0x01) && (ocr[3] == 0xAA))
        {
            // Leaving idle state.
            loggerOutputLine("sdDrvInit::SD_ACMD41");
            while (sdDrvCommand(SD_ACMD41, 1UL << 30));

            loggerOutputLine("sdDrvInit::SD_CMD58");
            if(sdDrvCommand(SD_CMD58, 0) == 0)
            {
                // Checking the CCS bit in the OCR.
                sdSPIReadBuffer(ocr, sizeof(ocr));
                cardType = (ocr[0] & 0x40) ? (CT_SD2 | CT_BLOCK) : CT_SD2;
            }
        }
    }
    else
    {
        // SDSC or MMC is detected.
        loggerOutputLine("sdDrvInit::SDSC/MMC");

        if(sdDrvCommand(SD_ACMD41, 0) <= 1)
        {
            // Card is SDSC card.
            loggerOutputLine("sdDrvInit::SDSC");

            cardType = CT_SD1;
            cmd = SD_ACMD41;

        }
        else
        {
            // Card is MMC.
            loggerOutputLine("sdDrvInit::MMC");

            cardType = CT_MMC;
            cmd = SD_CMD1;
        }

        // Wait until idle state is release.
        loggerOutputLine("sdDrvInit::Idle Release");
        while (sdDrvCommand(cmd, 0));

        // Check and set the read/write block size.
        loggerOutputLine("sdDrvInit::SD_CMD16");
        if(sdDrvCommand(SD_CMD16, 512) != 0)
        {
            // Block size set fail.
            cardType = 0;
        }
    }

#ifdef DEBUG_LOG
    char cardTypeStr[30];
    sprintf(cardTypeStr, "sdDrvInit::Type = %d", cardType);
    loggerOutputLine(cardTypeStr);
#endif

    sdSPIRelease();
    return (cardType != 0);
}

uint16_t sdDrvGetSectorSize(const struct block_device *dev)
{
    loggerOutputLine("sdDrvGetSectorSize");

    (void)dev;
    return 512;
}

int sdDrvReadSector(const struct block_device *bldev, uint32_t sector, uint32_t count, void *buf)
{
    int status = 0;
    (void)bldev;
    loggerOutputLine("sdDrvReadSector");

#ifdef DEBUG_LOG
    char readLog[30];
    sprintf(readLog, "sdDrvReadSector::sec = %d", sector);
    loggerOutputLine(readLog);
#endif

    sdSPISelect();

    if(!(cardType & CT_BLOCK))
    {
        // Convert to byte address.
        loggerOutputLine("sdDrvReadSector::Update sector");
        sector *= 512;
    }

    if(count == 1)
    {
        // Single block read.
        loggerOutputLine("sdDrvReadSector::Single block");

        if(sdDrvCommand(SD_CMD17, sector) == 0)
        {
            sdDrvGetBlock(buf, 512);
            status++;
        }
    }
    else
    {
        // Multipal block read.
        loggerOutputLine("sdDrvReadSector::Multipal block");

        // Start with multipal transfer command.
        if(sdDrvCommand(SD_CMD18, sector) == 0)
        {
            loggerOutputLine("sdDrvReadSector::SD_CMD18");

            do
            {
                if(sdDrvGetBlock(buf, 512) != 0)
                {
                    // Block read operation fail.
                    loggerOutputLine("sdDrvReadSector::Block read fail");
                    break;
                }

                buf += 512;
                status++;
            }
            while (--count);

            // End of transmission.
            loggerOutputLine("sdDrvReadSector::SD_CMD12");
            sdDrvCommand(SD_CMD12, 0);
        }
    }

    sdSPIRelease();
    return status;
}

int sdDrvWriteSector(const struct block_device *bldev, uint32_t sector, uint32_t count, const void *buf)
{
    int status = 0;
    (void)bldev;
    loggerOutputLine("sdDrvWriteSector");

#ifdef DEBUG_LOG
    char writeLog[30];
    sprintf(writeLog, "sdDrvWriteSector::sec = %d", sector);
    loggerOutputLine(writeLog);
#endif

    sdSPISelect();

    if(!(cardType & CT_BLOCK))
    {
        // Convert to byte address.
        loggerOutputLine("sdDrvWriteSector::Update sector");
        sector *= 512;
    }

    if(count == 1)
    {
        // Single block write.
        loggerOutputLine("sdDrvWriteSector::Single block");

        if(sdDrvCommand(SD_CMD24, sector) == 0)
        {
            loggerOutputLine("sdDrvWriteSector::Single block transfer");
            sdDrvSetBlock(buf, 512, TOKEN_SINGLE_WRITE);
            status++;
        }
    }
    else
    {
        // Multipal block write.
        loggerOutputLine("sdDrvWriteSector::Multipal block");

        if(cardType & CT_SDC)
        {
            // Perform bulk erase.
            loggerOutputLine("sdDrvWriteSector::SD_ACMD23");
            sdDrvCommand(SD_ACMD23, count);
        }

        // Start with multipal transfer command.
        if(sdDrvCommand(SD_CMD25, sector) == 0)
        {
            do
            {
                sdDrvSetBlock(buf, 512, TOKEN_MULTI_WRITE);
                buf += 512;
                status++;
            }
            while (--count);

            // End of bulk write operation.
            if (sdDrvSetBlock(0, 0, TOKEN_STOP_TRAN) != 0)
            {
                // Block write end operation fail.
                loggerOutputLine("sdDrvWriteSector::Block write end fail");
                status = 0;
            }
        }
    }

    sdSPIRelease();
    return status;
}

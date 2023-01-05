#include <stm32_SD.h>

    uint8_t cardType;
    uint8_t FileID=0;
    uint8_t DirID=0;
    int8_t CardStat=0;
    struct block_mbr_partition part;
	struct block_device dev;
	struct fat_vol_handle volHandle;
	struct fat_file_handle fileHandle;
    char statusBuff[30];
	char dirname[20];
	char filename[20];
	char buffer[100];

uint8_t sdSPIreadwrite(uint8_t data)
{
    while(SPI_SR(SPI1) & SPI_SR_BSY);
    SPI_DR(SPI1) = data;

    while(!(SPI_SR(SPI1) & SPI_SR_RXNE));
    return SPI_DR(SPI1);
}

void sdSPISelect()
{
    //gpio_clear(SDDRV_SPI_CS_PORT, SDDRV_SPI_CS);
	DigIo::SD_cs.Clear();
    // Wait until not busy.
    while(sdSPIreadwrite(0xFF) == 0);
}

void sdSPIRelease()
{
    //gpio_set(SDDRV_SPI_CS_PORT, SDDRV_SPI_CS);
  DigIo::SD_cs.Set();
    sdSPIreadwrite(0xFF);
}
void sdSPIWriteBuffer(const uint8_t *buf, int len)
{
    while(len--)
    {
        sdSPIreadwrite(*buf++);
    }
}

void sdSPIReadBuffer(uint8_t *buf, int len)
{
    while(len--)
    {
        *buf++ = sdSPIreadwrite(0xFF);
    }
}

static int sdDrvGetBlock(uint8_t *buf, int len)
{
    // Wait until SD card to getting into idle.
    while((*buf = sdSPIreadwrite(0xFF)) == 0xFF);

    if(*buf != TOKEN_SINGLE_READ)
    {
        // Invalid token.
        return -1;
    }

    sdSPIReadBuffer(buf, len);

    // Ignore CRC word.
    sdSPIreadwrite(0xFF);
    sdSPIreadwrite(0xFF);

    return 0;
}



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

static int sdDrvSetBlock(const uint8_t *buf, int len, uint8_t token)
{
    // Wait until SD card to getting into idle.
    while(sdSPIreadwrite(0xFF) != 0xFF);

    // Send token.
    sdSPIreadwrite(token);

    if (token != TOKEN_STOP_TRAN)
    {
        // Data token, and send data buffer.
        sdSPIWriteBuffer(buf, len);

        // Send dummy CRC word.
        sdSPIreadwrite(0xFF);
        sdSPIreadwrite(0xFF);

        // Response status of the transfer operation.
        return sdSPIreadwrite(0xFF);
    }

    return 0;
}

int sdDrvReadSector(const struct block_device *bldev, uint32_t sector, uint32_t count, void *buf)
{
    int status = 0;
    (void)bldev;


    sdSPISelect();

    if(!(cardType & CT_BLOCK))
    {
        // Convert to byte address.
        sector *= 512;
    }

    if(count == 1)
    {
        // Single block read.

        if(sdDrvCommand(SD_CMD17, sector) == 0)
        {
            sdDrvGetBlock(static_cast<uint8_t *>(buf), 512);
            status++;
        }
    }
    else
    {
        // Multipal block read.


        // Start with multipal transfer command.
        if(sdDrvCommand(SD_CMD18, sector) == 0)
        {

            do
            {
                if(sdDrvGetBlock(static_cast<uint8_t *>(buf), 512) != 0)
                {
                    // Block read operation fail.
                    break;
                }

                buf += 512;
                status++;
            }
            while (--count);

            // End of transmission.
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

    sdSPISelect();

    if(!(cardType & CT_BLOCK))
    {
        // Convert to byte address.
        sector *= 512;
    }

    if(count == 1)
    {
        // Single block write.

        if(sdDrvCommand(SD_CMD24, sector) == 0)
        {
            sdDrvSetBlock(static_cast<const uint8_t *>(buf), 512, TOKEN_SINGLE_WRITE);
            status++;
        }
    }
    else
    {
        // Multipal block write.

        if(cardType & CT_SDC)
        {
            // Perform bulk erase.
            sdDrvCommand(SD_ACMD23, count);
        }

        // Start with multipal transfer command.
        if(sdDrvCommand(SD_CMD25, sector) == 0)
        {
            do
            {
                sdDrvSetBlock(static_cast<const uint8_t *>(buf), 512, TOKEN_MULTI_WRITE);
                buf += 512;
                status++;
            }
            while (--count);

            // End of bulk write operation.
            if (sdDrvSetBlock(0, 0, TOKEN_STOP_TRAN) != 0)
            {
                // Block write end operation fail.
                status = 0;
            }
        }
    }

    sdSPIRelease();
    return status;
}

uint16_t sdDrvGetSectorSize(const struct block_device *dev)
{

    (void)dev;
    return 512;
}


int8_t stm32_SD::sdDrvInit()
{
    uint8_t ocr[4];
    uint8_t temp, cmd;
    cardType = 0;
     // Start SD card initialization with dummy write.
    for(temp = 0 ; temp < 80; temp++)
    {
        sdSPIreadwrite(0xFF);
    }

    if(sdDrvCommand(SD_CMD0, 0) != 1)
	{
            // Unable to reset the SD card.
            //Report a no sd init on webui
            Param::SetInt(Param::SDStatus,0);
            return -1;
	}

    // Identify the Version 1.x or Version 2.x SD card.
    if(sdDrvCommand(SD_CMD8, 0x1AA) == 1)
    {
        // SDHC is detected, try to get R7 response.
        Param::SetInt(Param::SDStatus,1);
        Param::SetInt(Param::SDType,1);
        sdSPIReadBuffer(ocr, sizeof(ocr));

        // Check card is support for 2.7V - 3.6V.
        if((ocr[2] == 0x01) && (ocr[3] == 0xAA))
        {
            // Leaving idle state.
            while (sdDrvCommand(SD_ACMD41, 1UL << 30));
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
        if(sdDrvCommand(SD_ACMD41, 0) <= 1)
        {
            // Card is SDSC card.
            Param::SetInt(Param::SDStatus,1);
            Param::SetInt(Param::SDType,2);
            cardType = CT_SD1;
            cmd = SD_ACMD41;

        }
        else
        {
            // Card is MMC.
            Param::SetInt(Param::SDStatus,1);
            Param::SetInt(Param::SDType,3);
            cardType = CT_MMC;
            cmd = SD_CMD1;
        }

        // Wait until idle state is release.
        while (sdDrvCommand(cmd, 0));

        // Check and set the read/write block size.
        if(sdDrvCommand(SD_CMD16, 512) != 0)
        {
            // Block size set fail.
            Param::SetInt(Param::SDStatus,0);
            Param::SetInt(Param::SDType,0);
            cardType = 0;
        }
    }

    sdSPIRelease();
    return (cardType != 0);
}

int8_t stm32_SD::OpenFatInit()
{

	// Initialize OpenFAT block device structure with SD driver functions.
	dev.read_sectors = sdDrvReadSector;
	dev.write_sectors = sdDrvWriteSector;
	dev.get_sector_size = sdDrvGetSectorSize;

	// Initialize partition block device.
	if(mbr_partition_init(&part, &dev, 0) == 0)
	{
           Param::SetInt(Param::SDMbr,1);
           CardStat++;
	}
	else
	{
            Param::SetInt(Param::SDMbr,0);
            CardStat--;
	}

	// Initialize and mount the FAT volume.
	if(fat_vol_init((struct block_device *)&part, &volHandle) == 0)
	{
            Param::SetInt(Param::SDfatT,volHandle.type);
            CardStat++;
	}
	else
	{
            CardStat--;
	}
            return CardStat;
}

void stm32_SD::SDTest1()
{
	// Start directory and file creation test (moved form example.c in original OpenFAT project).

	for(int i = 0; i < 5; i++)
	{
            sprintf(dirname, "Test%d", i);
            fat_mkdir(&volHandle, dirname);
            fat_chdir(&volHandle, dirname) == 0;

		for(int j = 0; j < 5; j++)
		{
                    sprintf(filename, "File%d", j);
                    fat_create(&volHandle, filename, O_WRONLY, &fileHandle) == 0;
                    fat_write(&fileHandle, buffer, sizeof(buffer)) == sizeof(buffer);
		}

            fat_chdir(&volHandle, "..") == 0;
	}

}

void stm32_SD::CreateDir()
{
            sprintf(dirname, "Logger%d", DirID);
            fat_mkdir(&volHandle, dirname);
            fat_chdir(&volHandle, dirname) == 0;
            Param::SetInt(Param::SDDir,DirID);
            DirID++;//Increment Dir ID for next time around
}

void stm32_SD::CreateFile()
{
            sprintf(filename, "Logfile%d.csv", FileID);
            fat_create(&volHandle, filename, O_WRONLY, &fileHandle) == 0;
            Param::SetInt(Param::SDFile,FileID);
            FileID++;//Increment Dir ID for next time around
}

void stm32_SD::WriteToFile(char *arr, uint8_t arr_S)
{
            fat_write(&fileHandle, arr,arr_S);
}


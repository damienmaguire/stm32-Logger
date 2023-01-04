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

#ifndef SDDRIVER_MAIN_HEADER
#define SDDRIVER_MAIN_HEADER

#include "../openfat/include/openfat.h"

void sdSPIInit();
int8_t sdDrvInit();

static uint8_t sdDrvCommand(uint8_t cmd, uint32_t arg);
static int sdDrvGetBlock(uint8_t *buf, int len);
static int sdDrvSetBlock(const uint8_t *buf, int len, uint8_t token);

int sdDrvReadSector(const struct block_device *bldev, uint32_t sector, uint32_t count, void *buf);
int sdDrvWriteSector(const struct block_device *bldev, uint32_t sector, uint32_t count, const void *buf);
uint16_t sdDrvGetSectorSize(const struct block_device *dev);

#endif /* SDDRIVER_MAIN_HEADER */
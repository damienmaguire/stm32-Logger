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

#ifndef SD_DEFINITIONS_HEADER
#define SD_DEFINITIONS_HEADER

// Command mnemonics for SD/MMC.
#define SD_CMD0     0   // GO_IDLE_STATE.
#define SD_CMD1     1   // SEND_OP_COND (MMC).
#define SD_CMD8     8   // SEND_IF_COND.
#define SD_CMD12    12  // STOP_TRANSMISSION.
#define SD_CMD16    16  // SET_BLOCKLEN.
#define SD_CMD17    17  // READ_SINGLE_BLOCK.
#define SD_CMD18    18  // READ_MULTIPLE_BLOCK.
#define SD_CMD24    24  // WRITE_BLOCK.
#define SD_CMD25    25  // WRITE_MULTIPLE_BLOCK.
#define SD_CMD55    55  // APP_CMD.
#define SD_CMD58    58  // READ_OCR.

#define SD_ACMD41   169 // SEND_OP_COND (SDC).
#define SD_ACMD23   151 // SET_WR_BLK_ERASE_COUNT (SDC).

#endif /* SD_DEFINITIONS_HEADER */
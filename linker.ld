/*
 * This file is part of the libopenstm32 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
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
 *	rom (rx)    : ORIGIN = 0x08000000, LENGTH = 128K
 	rom (rx)    : ORIGIN = 0x08020000, LENGTH = 128K
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Linker script for Olimex STM32-H405 (STM32F405RGT6, 1024K flash, 64K RAM). */

/* Define memory regions. */
MEMORY
{
 	rom (rx)    : ORIGIN = 0x08020000, LENGTH = 128K
	ram (rwx)   : ORIGIN = 0x20000000, LENGTH = 192K
}


/* Include the common ld script from libopenstm32. */
INCLUDE cortex-m-generic.ld

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
#include "logger.h"


#ifdef DEBUG_LOG
// Debug logs are enabled in this build configuration.

void loggerInit()
{

}

void loggerOutput(char *log)
{
    while(*log)
    {

        log++;
    }
}

void loggerOutputLine(char *log)
{
    loggerOutput(log);
    loggerOutput("\r\n");
}

#else
// Debug logs are disabled in this build configuration.

void loggerInit() {}
void loggerOutput(char *log) {}
void loggerOutputLine(char *log) {}

// End of DEBUG_LOG.
#endif

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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include "sddriver.h"
#include "sdspi.h"


// SPI peripheral to interface with SD card.
#define SDDRV_SPI               SPI1

// SPI related port.
#define SDDRV_SPI_PORT          GPIOA

// SPI chip selected (CS) related port.
#define SDDRV_SPI_CS_PORT       GPIOC

// SPI MOSI port pin.
#define SDDRV_SPI_MOSI          GPIO_SPI1_MOSI

// SPI MISO port pin.
#define SDDRV_SPI_MISO          GPIO_SPI1_MISO

// SPI SCK port pin.
#define SDDRV_SPI_SCK           GPIO_SPI1_SCK

// SPI chip select (CS) port pin.
#define SDDRV_SPI_CS            GPIO7

// SPI related clock configurations.
#define SDDRV_SPI_PORT_CLK_ID       RCC_GPIOB
#define SDDRV_SPI_CS_PORT_CLK_ID    RCC_GPIOA
#define SDDRV_SPI_CLK_ID            RCC_SPI2

void sdSPIInit()
{

}

void sdCardIntfInit()
{

}

uint8_t sdSPIreadwrite(uint8_t data)
{
    while(SPI_SR(SDDRV_SPI) & SPI_SR_BSY);
    SPI_DR(SDDRV_SPI) = data;

    while(!(SPI_SR(SDDRV_SPI) & SPI_SR_RXNE));
    return SPI_DR(SDDRV_SPI);
}

void sdSPISelect()
{
    gpio_clear(SDDRV_SPI_CS_PORT, SDDRV_SPI_CS);
	//DigIo::SD_cs.Clear();
    // Wait until not busy.
    while(sdSPIreadwrite(0xFF) == 0);
}

void sdSPIRelease()
{
    gpio_set(SDDRV_SPI_CS_PORT, SDDRV_SPI_CS);
  // DigIo::SD_cs.Set();
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

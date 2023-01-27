#ifndef stm32_usb_h
#define stm32_usb_h

/*  This library supports the on chip USB peripheral
Based on several libopencm3 examples and random key bashing by Damien Maguire.
*/
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/gpio.h>
#include <cstddef>
#include <cstring>
#include "params.h"

class stm32_usb
{

public:
static void usb_Startup();
static void usb_Poll();
static void usb_Status_Poll();
static void usb_Send(char *arr, uint8_t arr_S);
private:

};
#endif /* stm32_usb_h */








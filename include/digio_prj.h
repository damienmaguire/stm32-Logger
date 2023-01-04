#ifndef PinMode_PRJ_H_INCLUDED
#define PinMode_PRJ_H_INCLUDED

#include "hwdefs.h"

/* Here you specify generic IO pins, i.e. digital input or outputs.
 * Inputs can be floating (INPUT_FLT), have a 30k pull-up (INPUT_PU)
 * or pull-down (INPUT_PD) or be an output (OUTPUT)
*/

#define DIG_IO_LIST \
    DIG_IO_ENTRY(test_in,     GPIOB, GPIO5,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(led_out,     GPIOA, GPIO15, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(led_out2,     GPIOC, GPIO10, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(SD_cs,     GPIOC, GPIO7, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(CAN3_cs,     GPIOB, GPIO12, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(CAN4_cs,     GPIOC, GPIO6, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(CAN4_int,     GPIOC, GPIO9, PinMode::INPUT_FLT)      \
    DIG_IO_ENTRY(CAN3_int,     GPIOC, GPIO8, PinMode::INPUT_FLT)      \

#endif // PinMode_PRJ_H_INCLUDED

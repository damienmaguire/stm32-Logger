/*
 * This file is part of the stm32-template project.
 *
 * Copyright (C) 2020 Johannes Huebner <dev@johanneshuebner.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdint.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/can.h>
#include <libopencm3/stm32/iwdg.h>
#include "stm32_can.h"
#include "terminal.h"
#include "params.h"
#include "hwdefs.h"
#include "digio.h"
#include "hwinit.h"
#include "anain.h"
#include "param_save.h"
#include "my_math.h"
#include "errormessage.h"
#include "printf.h"
#include "stm32scheduler.h"
#include "stm32_usb.h"
#include "stm32_SD.h"



static Stm32Scheduler* scheduler;
static Can* can;
static Can* can2;
static bool CAN1Active=false;
static bool CAN2Active=false;
static bool CAN3Active=false;
static bool CAN4Active=false;
static bool extended=false;
static bool headerSent=false;
static bool newFile=false;
static void UpdateCanStats();
static uint32_t FrameCounter=0;
static uint32_t rtc_stamp=0;
static uint8_t BusId;
static uint8_t timer1Sec=10;
char Savvyheader[]=("Time,ID,Extended,Dir,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8\n\r");


//sample 100ms task
static void Ms100Task(void)
{
   //The following call toggles the LED output, so every 100ms
   //The LED changes from on to off and back.
   //Other calls:
   //DigIo::led_out.Set(); //turns LED on
   //DigIo::led_out.Clear(); //turns LED off
   //For every entry in digio_prj.h there is a member in DigIo
   DigIo::led_out.Toggle();
   DigIo::led_out2.Toggle();
   //The boot loader enables the watchdog, we have to reset it
   //at least every 2s or otherwise the controller is hard reset.
   iwdg_reset();
   //Calculate CPU load. Don't be surprised if it is zero.
   float cpuLoad = scheduler->GetCpuLoad();
   //This sets a fixed point value WITHOUT calling the parm_Change() function
   Param::SetFloat(Param::cpuload, cpuLoad / 10);

   //If we chose to send CAN messages every 100 ms, do this here.
   stm32_usb::usb_Status_Poll();
   UpdateCanStats();

   if(timer1Sec==0)//1 second routine
   {
   newFile=true;
   headerSent=false;
   timer1Sec=10;
   }
    timer1Sec--;
}

//sample 10 ms task
static void Ms10Task(void)
{
   //Set timestamp of error message
   ErrorMessage::SetTime(rtc_get_counter_val());

   if (DigIo::test_in.Get())
   {
      //Post a test error message when our test input is high
      ErrorMessage::Post(ERR_TESTERROR);
   }

    Param::SetInt(Param::FrameCtr,FrameCounter);//update total frames received....this may get big....
   //If we chose to send CAN messages every 10 ms, do this here.

}

static void ProcessCanData(uint32_t id, uint32_t data[2],uint8_t length,uint8_t BusId)
{
    if(id>0x7FF)extended=true;
    else extended=false;
    rtc_stamp=rtc_get_counter_val();
    uint32_t CanFrame[4]={id,length,data[0],data[1]};
    char *dataC = (char *)CanFrame;
    uint32_t idC = dataC[3] << 24 | dataC[2] << 16 | dataC[1] << 8 | dataC[0];//Merge id bytes
    char output_data[55];
    if(!headerSent)
    {
        sprintf(output_data,Savvyheader);
        headerSent=true;
    }
    else
    {
    sprintf(output_data, "%06d,%08x,%s,%s,%d,%d,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n\r",rtc_stamp,idC,extended?"True":"False","Rx",BusId,length,dataC[8],dataC[9],dataC[10],dataC[11],dataC[12],dataC[13],dataC[14],dataC[15]);

    }

    uint8_t output_data_size=sizeof(output_data);
    uint8_t Log_Modes = Param::GetInt(Param::Logging);
    switch(Log_Modes)
    {
    case 0:
    //Standby mode so ignote all
    break;

    case 1:
    //SD only
    stm32_SD::WriteToFile(output_data,output_data_size);
    break;

    case 2:
    //USB Only
    stm32_usb::usb_Send(output_data,output_data_size);
    break;

    case 3:
    //Both
    stm32_usb::usb_Send(output_data,output_data_size);
    stm32_SD::WriteToFile(output_data,output_data_size);
    break;

    default:

    break;
     }

    timer1Sec=10;//Reset out 1 second time out as long as we are still being called to process can.
}


static void CanCallback1(uint32_t id, uint32_t data[2],uint8_t length) //CAN1 Rx callback function
{
   //SendToUsb(id, data,length);
  // SendToSD(id, data,length);
   BusId=0;
   ProcessCanData(id,data,length,BusId);
   CAN1Active=true;
   FrameCounter++;

}

static void CanCallback2(uint32_t id, uint32_t data[2],uint8_t length) //CAN2 Rx callback function
{
   //SendToUsb(id, data,length);
   BusId=1;
   ProcessCanData(id,data,length,BusId);
   CAN2Active=true;
   FrameCounter++;
}

static void UpdateCanStats()
{
    if(CAN1Active) Param::SetInt(Param::CAN1Stat,1);
    else Param::SetInt(Param::CAN1Stat,0);

    if(CAN2Active) Param::SetInt(Param::CAN2Stat,1);
    else Param::SetInt(Param::CAN2Stat,0);

    if(CAN3Active) Param::SetInt(Param::CAN3Stat,1);
    else Param::SetInt(Param::CAN3Stat,0);

    if(CAN4Active) Param::SetInt(Param::CAN4Stat,1);
    else Param::SetInt(Param::CAN4Stat,0);

    CAN1Active=false;
    CAN2Active=false;
    CAN3Active=false;
    CAN4Active=false;

}

/** This function is called when the user changes a parameter */
void Param::Change(Param::PARAM_NUM paramNum)
{
   switch (paramNum)
   {
   default:
      //Handle general parameter changes here. Add paramNum labels for handling specific parameters
      break;
   }
}

//Whichever timer(s) you use for the scheduler, you have to
//implement their ISRs here and call into the respective scheduler
extern "C" void tim2_isr(void)
{
   scheduler->Run();
}

extern "C" int main(void)
{
   extern const TERM_CMD termCmds[];

   clock_setup(); //Must always come first
   rtc_setup();
   gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, AFIO_MAPR_CAN2_REMAP | AFIO_MAPR_CAN1_REMAP_PORTB);
   ANA_IN_CONFIGURE(ANA_IN_LIST);
   DIG_IO_CONFIGURE(DIG_IO_LIST);
   AnaIn::Start(); //Starts background ADC conversion via DMA
   write_bootloader_pininit(); //Instructs boot loader to initialize certain pins

   tim_setup(); //Sample init of a timer
   nvic_setup(); //Set up some interrupts
   parm_load(); //Load stored parameters
   spi1_setup();//SD card
   spi2_setup();//CAN3,4
   Stm32Scheduler s(TIM2); //We never exit main so it's ok to put it on stack
   scheduler = &s;
   //Initialize CAN1, including interrupts. Clock must be enabled in clock_setup()
   Can c(CAN1, (Can::baudrates)Param::GetInt(Param::canspeed1), true);
   Can c2(CAN2, (Can::baudrates)Param::GetInt(Param::canspeed2), true);
   //store a pointer for easier access
   can = &c;
   can2 = &c2;
    // Set up CAN 1 and 2 callback and messages to listen for
   c.SetReceiveCallback(CanCallback1);
   c2.SetReceiveCallback(CanCallback2);
 //  c.RegisterUserMessage(0x101);
  // c.RegisterUserMessage(0x102);
   //This is all we need to do to set up a terminal on USART3
   Terminal t(USART3, termCmds);

   stm32_usb::usb_Startup();

   //Up to four tasks can be added to each timer scheduler
   //AddTask takes a function pointer and a calling interval in milliseconds.
   //The longest interval is 655ms due to hardware restrictions
   //You have to enable the interrupt (int this case for TIM2) in nvic_setup()
   //There you can also configure the priority of the scheduler over other interrupts
   s.AddTask(Ms10Task, 10);
   s.AddTask(Ms100Task, 100);

   //backward compatibility, version 4 was the first to support the "stream" command
   Param::SetInt(Param::version, 4);
   Param::Change(Param::PARAM_LAST); //Call callback one for general parameter propagation

   //Now all our main() does is running the terminal
   //All other processing takes place in the scheduler or other interrupt service routines
   //The terminal has lowest priority, so even loading it down heavily will not disturb
   //our more important processing routines.

   ////////////////////////////////////////////////////////////////////////////////////////////
   //Here we start to play with the SD driver
   ///////////////////////////////////////////////////////////////////////////////////
	// Initialize SD card driver.
    if(stm32_SD::sdDrvInit())//init the sd card hardware
    {

    }
    stm32_SD::OpenFatInit();//wake up the sd card
    stm32_SD::CreateDir();//Setup a directory for logging to sd card
    stm32_SD::CreateFile();//create a new file on sd card for logging


   ///////////////////////////////////////////////////////////////////////////////////////////
   while(1)
   {
   t.Run();
   stm32_usb::usb_Poll();
   }


   return 0;
}


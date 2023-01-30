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
#include "stm32_sdio.h"

static Stm32Scheduler* scheduler;
static Can *can1, *can2;

extern "C" void __cxa_pure_virtual() { while (1); }
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
char Savvyheader[]=("Time Stamp,ID,Extended,Dir,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8\n\r");
char output_data[700];
char* pdata = output_data;
uint16_t output_data_size=0;
static uint16_t bufCnt=0;
static uint8_t queueCnt=0;
static bool dumpBuffer=false;
static uint16_t test9=0;
int z=0;



//sample 100ms task
static void Ms100Task(void)
{
   //The following call toggles the LED output, so every 100ms
    DigIo::led_out.Toggle();
    UpdateCanStats();
   iwdg_reset();
   //Calculate CPU load. Don't be surprised if it is zero.
   float cpuLoad = scheduler->GetCpuLoad();
   stm32_usb::usb_Status_Poll();
/*
      uint8_t bytes[8];
   bytes[0]=0xB9;
   bytes[1]=0x1C;
   bytes[2]=0x94;
   bytes[3]=0xAD;
   bytes[4]=0xC3;
   bytes[5]=0x15;
   bytes[6]=0x06;
   bytes[7]=0x63;
   Can::GetInterface(0)->Send(0x112, (uint32_t*)bytes,8);
   Can::GetInterface(1)->Send(0x212, (uint32_t*)bytes,8);

*/

   if(timer1Sec==0)//1 second routine. used as a canrx timeout to commence a new log and send savvy header
   {
    //DigIo::led_out2.Toggle();

   headerSent=false;
   timer1Sec=10;
   Param::SetInt(Param::opmode,0);
   }
    timer1Sec--;

}

//sample 10 ms task
static void Ms10Task(void)
{

Param::SetInt(Param::FrameCtr,FrameCounter);//update total frames received....this may get big....

   //If we chose to send CAN messages every 10 ms, do this here.

}

static void BufferDumper(void)
{
    Param::SetInt(Param::BufferS,output_data_size);
    uint8_t Log_Modes = Param::GetInt(Param::Logging);
    switch(Log_Modes)
    {
    case 0:
    //Standby mode so ignote all
    Param::SetInt(Param::opmode,0);
    break;

    case 1:
    //SD only
//    stm32_SD::WriteToFile(output_data,output_data_size);
    Param::SetInt(Param::opmode,1);
    newFile=false;//A new logging file will then spawn 1 sec afer the end of last msg received.
    break;

    case 2:
    //USB Only
    stm32_usb::usb_Send(output_data,output_data_size);
    dumpBuffer=false;
    Param::SetInt(Param::opmode,1);
    break;

    case 3:
    //Both

    stm32_usb::usb_Send(output_data,output_data_size);
    //    stm32_SD::WriteToFile(output_data,output_data_size);
    dumpBuffer=false;

    Param::SetInt(Param::opmode,1);
    newFile=false;//A new logging file will then spawn 1 sec afer the end of last msg received.
    break;

    default:

    break;
     }

}

static void ProcessCanData(uint32_t id, uint32_t data[2],uint8_t length,uint8_t BusId)
{
    if(id>0x7FF)extended=true;
    else extended=false;
//    rtc_stamp=rtc_get_counter_val();
    uint32_t CanFrame[4]={id,length,data[0],data[1]};
    char *dataC = (char *)CanFrame;
    uint32_t idC = dataC[3] << 24 | dataC[2] << 16 | dataC[1] << 8 | dataC[0];//Merge id bytes
    if(!headerSent)
    {
       // output_data_size=sprintf(output_data,Savvyheader);
        headerSent=true;
    }
    else
    {
     z=sprintf(pdata, "%06d,%08x,%s,%s,%d,%d,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\r\n",rtc_stamp,idC,extended?"True":"False","Rx",BusId,length,dataC[8],dataC[9],dataC[10],dataC[11],dataC[12],dataC[13],dataC[14],dataC[15]);
     pdata +=z;
    //output_data_size=strlen(output_data);
    bufCnt++;
    queueCnt++;
    if(queueCnt==2)
    {
    pdata=0;
    queueCnt=0;
    bufCnt=0;
    dumpBuffer=true;
    output_data_size=z;
    }
    }

    timer1Sec=10;//Reset our 1 second time out as long as we are still being called to process can.

}
static void CanCallback1(uint32_t id, uint32_t data[2],uint8_t length) //CAN1 Rx callback function
{
   BusId=0;
   ProcessCanData(id,data,length,BusId);
   CAN1Active=true;
   FrameCounter++;

}

static void CanCallback2(uint32_t id, uint32_t data[2],uint8_t length) //CAN2 Rx callback function
{
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
extern void parm_Change(Param::PARAM_NUM paramNum)
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
extern "C" void tim3_isr(void)
{
   scheduler->Run();
}

extern "C" int main(void)
{
   extern const TERM_CMD termCmds[];

   clock_setup(); //Must always come first
   //rtc_setup();
   ANA_IN_CONFIGURE(ANA_IN_LIST);
   DIG_IO_CONFIGURE(DIG_IO_LIST);
   AnaIn::Start(); //Starts background ADC conversion via DMA
   //write_bootloader_pininit(); //Instructs boot loader to initialize certain pins

   //tim_setup(); //Sample init of a timer
   nvic_setup(); //Set up some interrupts
   parm_load(); //Load stored parameters

   Stm32Scheduler s(TIM3); //We never exit main so it's ok to put it on stack
   scheduler = &s;
   //Initialize CAN1, including interrupts. Clock must be enabled in clock_setup()
   Can c1(CAN1, (Can::baudrates)Param::GetInt(Param::canspeed1));
   Can c2(CAN2, (Can::baudrates)Param::GetInt(Param::canspeed2));
   //store a pointer for easier access
   can1 = &c1;
   can2 = &c2;
   //TODO: Fix can lib to send dlc
    // Set up CAN 1 and 2 callback and messages to listen for
   c1.SetReceiveCallback(CanCallback1);
   c2.SetReceiveCallback(CanCallback2);

   Terminal t(USART3, termCmds);
   stm32_usb::usb_Startup();
   s.AddTask(Ms10Task, 10);
   s.AddTask(Ms100Task, 100);


   //backward compatibility, version 4 was the first to support the "stream" command
   Param::SetInt(Param::version, 4);
   parm_Change(Param::PARAM_LAST); //Call callback one for general parameter propagation

   while(1)
   {
      stm32_usb::usb_Poll();
      t.Run();
      if(dumpBuffer) BufferDumper();
   }


   return 0;
}

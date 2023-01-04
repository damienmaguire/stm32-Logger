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

/* This file contains all parameters used in your project
 * See main.cpp on how to access them.
 * If a parameters unit is of format "0=Choice, 1=AnotherChoice" etc.
 * It will be displayed as a dropdown in the web interface
 * If it is a spot value, the decimal is translated to the name, i.e. 0 becomes "Choice"
 * If the enum values are powers of two, they will be displayed as flags, example
 * "0=None, 1=Flag1, 2=Flag2, 4=Flag3, 8=Flag4" and the value is 5.
 * It means that Flag1 and Flag3 are active -> Display "Flag1 | Flag3"
 *
 * Every parameter/value has a unique ID that must never change. This is used when loading parameters
 * from flash, so even across firmware versions saved parameters in flash can always be mapped
 * back to our list here. If a new value is added, it will receive its default value
 * because it will not be found in flash.
 * The unique ID is also used in the CAN module, to be able to recover the CAN map
 * no matter which firmware version saved it to flash.
 * Make sure to keep track of your ids and avoid duplicates. Also don't re-assign
 * IDs from deleted parameters because you will end up loading some random value
 * into your new parameter!
 * IDs are 16 bit, so 65535 is the maximum
 */

 //Define a version string of your firmware here
#define VER 1.00.R

/* Entries must be ordered as follows:
   1. Saveable parameters (id != 0)
   2. Temporary parameters (id = 0)
   3. Display values
 */
//Next param id (increase when adding new parameter!): 3
//Next value Id: 2005
/*              category     name         unit       min     max     default id */
#define PARAM_LIST \
    PARAM_ENTRY(CAT_COMM,    canspeed1,    CANSPEEDS, 0,      4,      2,      1   ) \
    PARAM_ENTRY(CAT_COMM,    canspeed2,    CANSPEEDS, 0,      4,      2,      2   ) \
    PARAM_ENTRY(CAT_COMM,    canspeed3,    CANSPEEDS, 0,      4,      2,      3   ) \
    PARAM_ENTRY(CAT_COMM,    canspeed4,    CANSPEEDS, 0,      4,      2,      4   ) \
    PARAM_ENTRY(CAT_COMM,    Logging,      LOGMODES, 0,      4,      2,      5   ) \
    PARAM_ENTRY(CAT_COMM,    LogCtrl,      LOGCTRLS, 0,      4,      2,      6   ) \
    VALUE_ENTRY(opmode,      OPMODES, 2000 ) \
    VALUE_ENTRY(version,     VERSTR,  2001 ) \
    VALUE_ENTRY(SDStatus,    SDSTAT,  2002 ) \
    VALUE_ENTRY(SDType,      SDTYP,   2003 ) \
    VALUE_ENTRY(SDMbr,       SDMBR,   2005 ) \
    VALUE_ENTRY(SDfatT,      "dig",   2005 ) \
    VALUE_ENTRY(USBStat,     USBSTT,   2006 ) \
    VALUE_ENTRY(CAN1Stat,    CANSTT,   2007 ) \
    VALUE_ENTRY(CAN2Stat,    CANSTT,   2008 ) \
    VALUE_ENTRY(CAN3Stat,    CANSTT,   2009 ) \
    VALUE_ENTRY(CAN4Stat,    CANSTT,   2010 ) \
    VALUE_ENTRY(SDDir,       "dir",   2011 ) \
    VALUE_ENTRY(SDFile,      "fil",   2012 ) \
    VALUE_ENTRY(FrameCtr,    "Frm",   2013 ) \
    VALUE_ENTRY(cpuload,     "%",     2004 )


/***** Enum String definitions *****/
#define OPMODES      "0=Off, 1=Logging"
#define SDSTAT      "0=None, 1=Det"
#define SDMBR       "0=Fail, 1=Ok"
#define SDTYP       "0=None, 1=SDHC, 2=SDSC, 3=MMC"
#define USBSTT       "0=Closed, 1=Open"
#define CANSTT       "0=Idle, 1=Actty"
#define CANSPEEDS    "0=125k, 1=250k, 2=500k, 3=800k, 4=1M"
#define LOGMODES      "0=Standby, 1=SDonly, 2=USBonly, 3=Both"
#define LOGCTRLS      "0=Off, 1=AutoSD, 2=Manual"
#define CAT_COMM     "Communication"

#define VERSTR STRINGIFY(4=VER-Log)

/***** enums ******/


enum _canperiods
{
   CAN_PERIOD_100MS = 0,
   CAN_PERIOD_10MS,
   CAN_PERIOD_LAST
};

enum _modes
{
   MOD_OFF = 0,
   MOD_RUN,
   MOD_LAST
};

//Generated enum-string for possible errors
extern const char* errorListString;


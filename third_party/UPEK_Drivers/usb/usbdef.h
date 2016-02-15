/**
 * @file usbdef.h
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */

#ifndef _STUSB_H_
#define _STUSB_H_



#ifdef __cplusplus
extern "C" {
#endif


#include "usberr.h"



#define MSB(dword)		(BYTE)(((DWORD)dword >> 16) & 0xff)
#define MID(dword)		(BYTE)(((DWORD)dword >> 8) & 0xff)
#define LSB(dword)		(BYTE)((DWORD)dword & 0xff)

#define GRAB_TYPE_SETTINGS_GRAB 1   // cycles through the settings table after every 1024 pixels
#define GRAB_TYPE_NEW_GRAB      2   // packet count to eliminate dangling packet

#define ST_RESET 0xFE
#define ST_DATAFLOW 0xFD
#define ST_BESTSETT 0xFC  
#define ST_PARAMETER_SET 0xA0

#define ST_TOUCH_GET_REG_BLOCK              2   // read a set of TC registers
#define ST_TOUCH_SET_REG_BLOCK              3   // write a set of TC registers
#define ST_TOUCH_GET_REGISTER               5
#define ST_TOUCH_SET_REGISTER               6
#define ST_TOUCH_LOAD_TABLE                 7   // settings table
#define ST_TOUCH_SET_WINDOW                 8   // Set window command
#define ST_TOUCH_SET_REPAIR                 9   // repair info 
#define ST_TOUCH_SET_NV                     10  // write to Non Volatile Memory
#define ST_TOUCH_GET_NV                     11  // read from Non Volatile Memory
#define ST_TOUCH_NEW_GRAB                   14  // Start image grab of known size.
#define ST_TOUCH_ERASE_SECTOR               20  // erase an NVM sector
#define ST_TOUCH_BULK_ERASE                 21  // erase entire NVM
#define ST_TOUCH_GET_NV_HUGE                22  // read huge area of NVM 
#define ST_TOUCH_GET_CHALLENGE              24  // get challenge (stage 1 of encryption)
#define ST_TOUCH_SESSION_KEY                25  // get Hash (stage 2 of encryption)
#define ST_TOUCH_GET_FLASH_ID               26  // read ID from external flash
#define ST_TOUCH_SET_ENCRYPTION             27  // turn encryption on

#define LANGID_US_ENGLISH	0x0409	

#define CYPRESS_NVM_SECTOR0_START           0x000000    // start of sector 0
#define CYPRESS_NVM_SECTOR0_END             0x007FFF    // end of sector 0
#define CYPRESS_NVM_SECTOR1_START           0x008000    // start of sector 1
#define CYPRESS_NVM_SECTOR1_END             0x00FFFF    // end of sector 1
#define CYPRESS_NVM_SECTOR2_START           0x010000    // start of sector 2
#define CYPRESS_NVM_SECTOR2_END             0x017FFF    // end of sector 2
#define CYPRESS_NVM_SECTOR3_START           0x018000    // start of sector 3
#define CYPRESS_NVM_SECTOR3_END             0x01FFFF    // end of sector 3

#define STM32_NVM_SECTOR0_START             0x000000    // start of sector 0
#define STM32_NVM_SECTOR0_END               0x0007FF    // end of sector 0
#define STM32_NVM_SECTOR1_START             0x000800    // start of sector 1
#define STM32_NVM_SECTOR1_END               0x0087FF    // end of sector 1
#define STM32_NVM_SECTOR2_START             0x008800    // start of sector 2
#define STM32_NVM_SECTOR2_END               0x0107FF    // end of sector 2
#define STM32_NVM_SECTOR3_START             0x010800    // start of sector 3
#define STM32_NVM_SECTOR3_END               0x0187FF    // end of sector 3
#define STM32_NVM_SECTOR4_START             0x018800    // start of sector 4
#define STM32_NVM_SECTOR4_END               0x0207FF    // end of sector 4


int GetDeviceList(DeviceInfoStruct *p_device_info);

int InitializeST_USB(char * CreateFileName, HANDLE the_event);

int SendSettings( BYTE *SettingsTablePtr, int NumSettings);

int SendCmdTable( int command, int	StartAddress,BYTE *bptr, int TableSize );

int CleanUpST_USB();
int GrabImageST_USB(unsigned char *Buffer, unsigned long BufferSize, int imagetype);

int Read_Physical_Reg(int PhysRegNum, int *Data);
int Write_Physical_Reg(int PhysRegNum, BYTE *Data, int NumBytes);

int EraseSector(DWORD StartAddress);
int BulkErase();



/* End of extern "C" {  declaration for C++ callers */
#ifdef __cplusplus
}
#endif

#endif //_STUSB_H_

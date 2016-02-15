/**
* @file usbl.h
*
* Usb layer functions
*
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */

#ifndef __USBL_H__
#define __USBL_H__

#include "lltypes.h"
#include "defines.h"
#include <usb.h>

// header types
typedef void* USB_DEVICE;
typedef int USB_STATUS;
typedef short USB_COMMAND;

// USB_STATUS values

// other constants
#define USB_TIMEOUT_NONE 1
#define USB_TIMEOUT_INFINITE 0x7FFFFFFF
#define USB_MAX_BULK_PACKET_SIZE 64

int UsbReadData(LL_CONN_HANDLE handle, uint8 *pBuffer, uint32 dwLength);
int UsbWriteData(LL_CONN_HANDLE handle, uint8 *pBuffer, uint32 dwLength);
int UsbOpenDevice(PLL_CONN_HANDLE phandle, PLL_CONN_PARAMS pParameters);
int UsbCloseDevice(PLL_CONN_HANDLE phandle);
//int UsbEnumerateDevices(IN PLL_CONN_PARAMS pParameters,OUT PT_DEVICE_LIST **ppList);


#endif // __USBL_H__

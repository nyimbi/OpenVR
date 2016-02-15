/**
 * @file deviceList.h
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef _DEVICELIST_H_
#define _DEVICELIST_H_


//-------------------------------- Constants --------------------------------
/**
 * @defgroup devicelistparams Device List Parameters
 * @{
 */
#define MAX_NUM_READERS         5   ///< Max number of readers to be supported
#define MAX_USB_DEVICE_STR_LEN  256 ///< Maximum length of reader information strings
/**  @} */


//-------------------------------- Structures -------------------------------
/// Information structure for a device
typedef struct
{
    char CreateFileName[MAX_USB_DEVICE_STR_LEN];
    int  BUSID;
    char ManufactureString[MAX_USB_DEVICE_STR_LEN];
    char DeviceString[MAX_USB_DEVICE_STR_LEN];
    char FriendlyNameString[MAX_USB_DEVICE_STR_LEN];
} DeviceInfoEntry;

/// List structure for devices
typedef struct
{
    int NumDevices;
    DeviceInfoEntry device_entry[MAX_NUM_READERS];
} DeviceInfoStruct;

#endif  // #ifndef _DEVICELIST_H_

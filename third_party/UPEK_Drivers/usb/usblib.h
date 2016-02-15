/**
 * @file usblib.h
 * USB function prototypes
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */

/////////////////////////////////////////////////////////
// function prototypes 
/////////////////////////////////////////////////////////

#define __stdcall

/* Assume C declarations for C++ callers */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Retrieves list of devices connected through USB
 *
 * @param p_device_info Receives information on all devices 
 * connected by usb.  Must be pre-allocated in memory.
 *
 * @return Status of command
 */
int __stdcall OnGetDeviceListUSB(DeviceInfoStruct *p_device_info);

/**
 * @brief Initializes USB device with device handle name @a createFileName
 *
 * @param createFileName Device handle name or @c NULL
 * @param the_event Event to tell TCI when a finger arrives.
 *
 * @return Status of command
 */
int __stdcall OnSelectUSB(char *createFileName, HANDLE the_event);

/**
 * @brief Reads from physical registers
 *
 * @param RegNum Register number to read (physical)
 * @param DataI2C Buffer to receive data.  Must be pre-allocated in memory.
 *
 * @return Status of command
 */
int __stdcall OnReadUSB(int RegNum, int *DataI2C);

/**
 * @brief Closes open USB devices
 *
 * @return Status of command
 */
int __stdcall OnCloseUSB();

/**
 * @brief Writes to physical registers
 *
 * @param RegNum Register number to write (physical)
 * @param DataI2C Buffer containing data to write
 *
 * @return Status of command
 */
int __stdcall OnWriteUSB(int RegNum, int DataI2C);

/**
 * @brief Grabs an image from a USB device
 *
 * @param Buffer Data buffer to receive the grabbed data
 * @param BufferSize Size of data buffer
 * @param imagetype Type of image grab to perform - @ref grabtypes
 *
 * @return Status of command
 */
int __stdcall OnGrabUSB(unsigned char *Buffer, unsigned long BufferSize, int imagetype);

/**
 * @param bptr Settings lookup table
 * @param NumSetts Number of settings
 *
 * @return Status of command
 */
int __stdcall OnSendSettingsUSB( unsigned char * bptr,				// settings lookup table
							 int NumSetts);					// number of settings

/**
 * @brief This is a general purpose function used to pass a command code and a table of 
 * byte data to the USB uP. 
 *
 * This can replace the dedicated @ref OnSendSettingsUSB function in TCI if the additional
 * command byte is passed in.
 *
 * @param command Command code passed to USB
 * @param StartAddress Destination Address
 * @param bptr Pointer to byte table
 * @param TableSize Number of bytes
 *
 * @return Status of command
 */
int __stdcall OnSendCmdTableUSB( int command,			// command code passed to USB
					int	StartAddress,		// destination address 
					unsigned char * bptr,	// pointer to byte table
					int TableSize);			// number of bytes

#ifdef __cplusplus
}
#endif
/////////////////////////////////////////////////////////

/**
 * @file usberr.h
 * USB driver error codes
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */


// USB driver error codes


#define   ST_USB_ERR_NONE 0				// no error
#define   ST_USB_ERR_DRVOPEN -101		// unable to open USB driver
#define   ST_USB_ERR_USBRD -102			// USB failure during Bulk read
#define   ST_USB_ERR_USBWR -103			// USB failure during Bulk write
#define   ST_USB_ERR_BUFFER -104		// Image size mismatch between chip and application
#define   ST_USB_ERR_IMAGE -105			// Incomplete image.
#define   ST_USB_ERR_RANGE -106			// register value out of range
#define   ST_USB_ERR_INVALID -107		// target unable to read register value
#define   ST_USB_ERR_CRYPT -108			// target unable to read register value
#define   ST_USB_ERR_SC_RET -109		// unexpected smartcard return value 
#define   ST_USB_ERR_SC_NO_CARD -110	// SmartCard did not answer reset 
#define   ST_USB_ERR_HANDLE -111		// The device handle seems to be invalid 

#define   ST_USB_ERR_TIMEOUT -112			// timeout
#define   ST_USB_ERR_DEVICE_NOT_FOUND -113	// no device found
#define   ST_USB_ERR_COMM -114							// communication error
#define   ST_USB_ERR_MEMORY_MALLOC_FAIL -115	// memory allocation failure
#define   ST_USB_ERR_EX_ACCESS_VIOLATION -116	// access violation
#define   ST_USB_ERR_GENERAL_ERROR -117				// general error
#define   ST_USB_ERR_UNSUPPORTED_SPEED -118		// speed is unsupported

#define   ST_USB_HASH_MISMATCH -201
#define   ST_USB_INVALID_PACKET -202

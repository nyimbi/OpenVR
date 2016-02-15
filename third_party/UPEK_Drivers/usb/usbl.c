/**
 * @file usbl.c
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
#include "Tdbg.h"
#include "usbl.h"
#include "usb.h"
#include "usbtypes.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "crc.h"
#include "usberr.h"

TDBG_DEFINE_AREA(USBL);
TDBG_DEFINE_AREA(USBH);
TDBG_DEFINE_AREA(USBLDUMP);
#define TTRACE_LEAVE_STATUS() TTRACE_LEAVE(("%s() -> %li",__FUNCTION__,status))


// ---- USB IDs ----------------------------

#define USB_ST_VID              0x0483          // ST vendor ID
#define USB_UPEK_VID            0x147E          // UPEK vendor ID
#define USB_TC_PID              0x2015          // blue readers product ID
#define USB_TFM_PID             0x2016          // TFM/ESS product ID
#define USB_COMBO_PID           0x2017          // Combo product ID
#define USB_BIOSIMKEY_PID       0x2018          // BiosimKey product ID
#define USB_SONLY_PID           0x1000          // Sonly product ID
#define USB_CYPRESS_PID         0x3000          // TCEFA1 or TCEFB1
#define USB_STM32_PID           0x3001          // Eikon Touch


#define OS_ERROR_DEVICE_NOT_FOUND -19

#define OS_ERROR_TIMEOUT           -110 

// timeout for bulk transfer
#define USB_BULK_TIMEOUT 5000

// input bulk endpoint
#define USB_BULK_IN 0x82
#define USB_BULK_IN_STM32 0x81

// output bulk endpoint
#define USB_BULK_OUT 0x03
#define USB_BULK_OUT_STM32 0x02

extern BOOL UseSTM32;


static void InvalidateHandle(LL_CONN_HANDLE handle)
{
    handle->mhHandle=0;
}

static int UsbTranslateStatus(IN int status)
{
    TASSERT(status<=0);
    if(status >= 0) return 0;
    switch(status)
    {
    case OS_ERROR_DEVICE_NOT_FOUND:
            return ST_USB_ERR_DEVICE_NOT_FOUND;
    case OS_ERROR_TIMEOUT:
            return ST_USB_ERR_TIMEOUT;
    default:
            return ST_USB_ERR_COMM;
    }
}


#define USB_ST_VID              0x0483          // ST vendor ID
#define USB_UPEK_VID            0x147E          // UPEK vendor ID
#define USB_TC_PID              0x2015          // blue readers product ID
#define USB_TFM_PID             0x2016          // TFM/ESS product ID
#define USB_COMBO_PID           0x2017          // Combo product ID
#define USB_BIOSIMKEY_PID       0x2018          // BiosimKey product ID
#define USB_SONLY_PID           0x1000          // Sonly product ID


static struct usb_bus *UsbDoOSEnumerateDevices()
{
    struct usb_bus * pResult=NULL;
    TTRACE_ENTER(USBL,TDBG_TRACE,("UsbDoEnumerateDevices()"));
    long status=0;
    errno=0;
    // search device
    TTRACE(USBL,TDBG_TRACE,("usb_init()"));
    usb_init();
    TTRACE(USBL,TDBG_TRACE,("usb_init() ->"));

    TTRACE(USBL,TDBG_TRACE,("usb_find_buses()"));
    status = usb_find_busses();
    TTRACE(USBL,TDBG_TRACE,("usb_find_buses() -> %li",status));
    if(status<0)
        goto lbExit;

    TTRACE(USBL,TDBG_TRACE,("usb_find_devices()"));
    status = usb_find_devices();
    TTRACE(USBL,TDBG_TRACE,("usb_find_devices() -> %li",status));
    if(status<0)
        goto lbExit;

    TTRACE(USBL,TDBG_TRACE,("usb_get_devices()"));
    pResult = usb_get_busses();
    TTRACE(USBL,TDBG_TRACE,("usb_get_devices() ->0x%p",pResult));
lbExit:
    TTRACE_LEAVE(("UsbDoOSEnumerateDevices() -> 0x%p",pResult));
    return pResult;
};


int UsbOpenDevice(PLL_CONN_HANDLE phandle, PLL_CONN_PARAMS pParameters)
{
    int status;
    struct usb_bus *pBusses, *pBus;
    struct usb_device *pDevice;
    usb_dev_handle *pDeviceHandle;

    TASSERT(pParameters);
    TASSERT(pParameters->mConectionType==LL_CONN_USB);
    TTRACE_ENTER(USBL,TDBG_TRACE,("UsbOpenDevice(0x%p,0x%p(=0x%lx,0x%lx),\"%s\",%lu)",phandle,pParameters,pParameters->mUSB.mdwVid,pParameters->mUSB.mdwPid,
        pParameters->mUSB.msDeviceName,pParameters->mdwTimeout));
    

    TASSERT(phandle);
    pBusses = UsbDoOSEnumerateDevices();

    pBus = pBusses;
    while(pBus)
    {
        pDevice = pBus->devices;
        while(pDevice)
        {
            if ((pDevice->descriptor.idVendor == USB_UPEK_VID) && (pDevice->descriptor.idProduct == USB_STM32_PID)) //STM32
            {
                pParameters->mUSB.mdwPid = pDevice->descriptor.idProduct;
                goto lbFound;
            }
            pDevice = pDevice->next;
        }
        pBus = pBus->next;
    }
    
    status =  ST_USB_ERR_DEVICE_NOT_FOUND;
    goto lbExit;



lbFound:

    *phandle=(LL_CONN_HANDLE)malloc(sizeof(LL_CONN_STRUCT));
    if(!(*phandle))
    {
        status = ST_USB_ERR_MEMORY_MALLOC_FAIL;
        goto lbExit;
    }
    memset((*phandle),0,sizeof(LL_CONN_STRUCT));
    memcpy(&((*phandle)->mConnParams),pParameters,sizeof(LL_CONN_PARAMS));
    
    // open device
    TTRACE(USBL,TDBG_TRACE,("usb_open(0x%p)",pDevice));
    pDeviceHandle = usb_open(pDevice);
    TTRACE(USBL,TDBG_TRACE,("usb_open() -> 0x%p",pDeviceHandle));
    if( !pDeviceHandle )
    {
        status = ST_USB_ERR_COMM;
        goto error;
    }

   
    TTRACE(USBL,TDBG_TRACE,("usb_set_configuration(0x%p,1)",pDeviceHandle));
    status = usb_set_configuration(pDeviceHandle, 1);
    TTRACE(USBL,TDBG_TRACE,("usb_set_configuration() -> %li",status));
    

    if(status == 0)
    {
        // claim interface
        TTRACE(USBL,TDBG_TRACE,("usb_claim_interface(0x%p,0)",pDeviceHandle));
        status = usb_claim_interface(pDeviceHandle, 0);
        TTRACE(USBL,TDBG_TRACE,("usb_claim_interface() -> %li",status));
    }
    if( status < 0 )
    {
        usb_close(pDeviceHandle);
        goto error;
    }

    (*phandle)->mhHandle = pDeviceHandle;
    status = ST_USB_ERR_NONE;
    goto lbExit;

error:
    free(*phandle);*phandle=NULL;
    status = UsbTranslateStatus(status);

lbExit:;
    TTRACE_LEAVE_STATUS();
    return status;

}

int UsbCloseDevice(PLL_CONN_HANDLE phandle)
{
    int status = 0;
    LL_CONN_HANDLE handle=NULL;
    
    TTRACE_ENTER(USBL,TDBG_TRACE,("UsbCloseDevice(0x%p)",phandle));
    TASSERT(phandle);
    handle=*phandle;
    TASSERT(handle);

    if(!(handle->mhHandle))
    {
        TTRACE(USBL,TDBG_WARN,("Device was removed, skipping..."));
    }
    else
    {
        // release interface
        TTRACE(USBL,TDBG_TRACE,("usb_release_interface(0x%p)",handle->mhHandle));
        status = usb_release_interface((usb_dev_handle *)(handle->mhHandle), 0);
        TTRACE(USBL,TDBG_TRACE,("usb_release_interface() -> %li",status));

        // close device
        TTRACE(USBL,TDBG_TRACE,("usb_close(0x%p)",handle->mhHandle));
        status = usb_close((usb_dev_handle *)(handle->mhHandle));
        TTRACE(USBL,TDBG_TRACE,("usb_close() -> %li",status));

    }
    handle->mhHandle=0;
    free(handle);
    phandle=NULL;
    TTRACE_LEAVE_STATUS();
    return status;
}


// reads data from the device
int UsbReadData(LL_CONN_HANDLE handle, uint8 *pBuffer, uint32 dwLength)
{
    int status;
    TTRACE_ENTER(USBL,TDBG_TRACE,("UsbReadDevice(0x%p,0x%p,%lu)",handle,pBuffer,dwLength));

    TASSERT(handle);
    TASSERT(pBuffer);
    TASSERT(dwLength);
    if(!handle->mhHandle)
    {
        TTRACE(USBL,TDBG_WARN,("Invalid handle, broken session?"));
        status=ST_USB_ERR_COMM;
        goto lbExit;
    }
    if(UseSTM32)
        status = usb_bulk_read((usb_dev_handle *)(handle->mhHandle), USB_BULK_IN_STM32, (char *)pBuffer,dwLength, handle->mConnParams.mdwTimeout/*USB_BULK_TIMEOUT*/);
    else
        status = usb_bulk_read((usb_dev_handle *)(handle->mhHandle), USB_BULK_IN, (char *)pBuffer,dwLength, handle->mConnParams.mdwTimeout/*USB_BULK_TIMEOUT*/);
    TTRACE(USBL,TDBG_TRACE,("usb_bulk_read() -> %li", status));
    TDUMP_EX(USBLDUMP,TDBG_TRACE,"Read data",pBuffer,status);
    if(OS_ERROR_DEVICE_NOT_FOUND==status)
    {
        InvalidateHandle(handle);
    }

    if( status < 0 )
    {
        status=UsbTranslateStatus(status);
        goto lbExit;
    }
        
    if(status<dwLength)    
    {
        TTRACE(USBL,TDBG_ERROR,("Data shorter (%lu) than expected (%lu)",status,dwLength));
        status=ST_USB_ERR_COMM;
        goto lbExit;
    }

        status = ST_USB_ERR_NONE;


lbExit:
    TTRACE_LEAVE_STATUS();
    return status;
}


// writes data to the device
int UsbWriteData(LL_CONN_HANDLE handle, uint8 *pBuffer, uint32 dwLength)
{
    int status;
    
    TTRACE_ENTER(USBL,TDBG_TRACE,("UsbWriteData(0x%p,0x%p,%lu)",handle,pBuffer,dwLength));

    TASSERT(handle);
    TASSERT(pBuffer);
    TASSERT(dwLength);
    if(!handle->mhHandle)
    {
        TTRACE(USBL,TDBG_WARN,("Invalid handle, broken session?"));
        status=ST_USB_ERR_COMM;
        goto lbExit;
    }

    TDUMP_EX(USBLDUMP,TDBG_TRACE,"Write data",pBuffer,dwLength);

    if(UseSTM32)
        status = usb_bulk_write((usb_dev_handle *) (handle->mhHandle), USB_BULK_OUT_STM32, (char *)pBuffer, dwLength, handle->mConnParams.mdwTimeout);
    else
        status = usb_bulk_write((usb_dev_handle *) (handle->mhHandle), USB_BULK_OUT, (char *)pBuffer, dwLength, handle->mConnParams.mdwTimeout);
    TTRACE(USBL,TDBG_TRACE,("usb_bulk_write() -> %li", status));
    if(OS_ERROR_DEVICE_NOT_FOUND==status)
    {
        InvalidateHandle(handle);
    }

    if(status!=dwLength)
    {
        if(status>=0)
        {
            TTRACE(USBL,TDBG_ERROR,("Data shorter (%lu) than expected (%lu)",status,dwLength));
            status = ST_USB_ERR_COMM;
            goto lbExit;
        }
        status = UsbTranslateStatus(status);
        goto lbExit;
    }

    status = ST_USB_ERR_NONE;

lbExit:
    TTRACE_LEAVE_STATUS();    
    return status;
}





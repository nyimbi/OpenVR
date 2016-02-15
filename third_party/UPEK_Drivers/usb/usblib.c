/**
 * @file usblib.c
 * USB LIB function
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "deviceList.h"
#include "usbtypes.h"
#include "usblib.h"
#include "usbdef.h"
#include "usbl.h"

#define DEFAULT_VENDOR_ID           0x0483
#define DEFAULT_PRODUCT_ID          0x2015
#define DEFAULT_DEVICE_TYPE         0x0001

/////////////////////////////////////////////////////////////////////////////////////////
///////////            Low level USB functions                              /////////////
/////////////////////////////////////////////////////////////////////////////////////////

BOOL UseSTM32        = FALSE;        // TRUE if using STM32

//Global USB handle
static LL_CONN_HANDLE gMy_usb_handle;
static LL_CONN_PARAMS gMy_usb_parameters;

int USBBulkDataWrite(unsigned char *pBuffer, unsigned long BufferSize, int timeout) {
    int retval;
    retval = UsbWriteData(gMy_usb_handle, (uint8*)pBuffer, BufferSize);
    if (retval)
        return(ST_USB_ERR_USBWR);
    else
        return(ST_USB_ERR_NONE);
}

int USBBulkDataRead(unsigned char *pBuffer, unsigned long BufferSize, int timeout) {
    int retval;
    retval = UsbReadData(gMy_usb_handle, (uint8*)pBuffer, BufferSize);
    if (retval)
        return(ST_USB_ERR_USBRD);
    else
        return(ST_USB_ERR_NONE);
}

#define USB_STM32_PID           0x3001          // Eikon Touch

int InitializeST_USB(char *CreateFileName, HANDLE the_event) {
    int  retval = ST_USB_ERR_NONE;
    BYTE tempbyte;

    if (the_event != NULL) {
        return(ST_USB_ERR_HANDLE);
      }

    // setup USB parameters
    memset(&gMy_usb_parameters, 0, sizeof(LL_CONN_PARAMS));
    gMy_usb_parameters.mConectionType = LL_CONN_USB;
    gMy_usb_parameters.mdwTimeout = DEFAULT_TIMEOUT;

    // look for the first device in the old way,
    // by searching through all the name space
    // and return the name.
    retval = UsbOpenDevice(&gMy_usb_handle, &gMy_usb_parameters);

    if (retval != ST_USB_ERR_NONE) {
        return (ST_USB_ERR_DRVOPEN);
    }

    UseSTM32 = FALSE;
    if (gMy_usb_parameters.mUSB.mdwPid == USB_STM32_PID)
    {
        UseSTM32 = TRUE;
    }

    // issue a TouchChip reset...
    tempbyte = 0;

    retval = Write_Physical_Reg(ST_RESET, &tempbyte, 1);

    if (retval != ST_USB_ERR_NONE) {
        UsbCloseDevice(&gMy_usb_handle); // release the device
        return(retval);
    }

    return(ST_USB_ERR_NONE);
}

int CleanUpST_USB() {
    BYTE tempbyte;
    if(UseSTM32) {
        tempbyte = 0xFF;
        Write_Physical_Reg(0xF7, &tempbyte, 1);
    }
    UsbCloseDevice(&gMy_usb_handle); // release the device
    return(ST_USB_ERR_NONE);
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////            The following code are PP related                    /////////////
/////////////////////////////////////////////////////////////////////////////////////////

int Read_Physical_Reg(int PhysRegNum, int *Data) {
    int retval;

    unsigned char buffer64[64];
    buffer64[0] = (unsigned char)(ST_TOUCH_GET_REG_BLOCK | 0x80);
    buffer64[1] = 0;
    buffer64[2] = 0;
    buffer64[3] = 0;
    buffer64[4] = 1;
    buffer64[5] = PhysRegNum;

    retval = USBBulkDataWrite(buffer64, 64, 1000);

    if (retval != ST_USB_ERR_NONE) {
        return(retval);
    }

    retval = USBBulkDataRead(buffer64, 64, 1000);

    if (retval != ST_USB_ERR_NONE) {
        return(retval);
    }

    if (buffer64[0] != ST_TOUCH_GET_REG_BLOCK) {
        return ST_USB_ERR_INVALID;
    }

    // copy data from buffer to Data
    *Data = buffer64[1];
    return ST_USB_ERR_NONE;
}

// USB Write to hardware using Bulk Transfers
int Write_Physical_Reg(int PhysRegNum, unsigned char *Data, int NumBytes) {
    int retval;

    unsigned char buffer64[64];

    buffer64[0] = (unsigned char)ST_TOUCH_SET_REG_BLOCK;
    buffer64[1] = 0;
    buffer64[2] = 0;
    buffer64[3] = 0;
    buffer64[4] = 2;
    buffer64[5] = PhysRegNum;
    buffer64[6] = *Data;

    retval = USBBulkDataWrite(buffer64, 64, 1000);
    return(retval);
}

// erase a sector from flash NVM
int EraseSector(DWORD StartAddress) {
    BYTE buffer64[64];
    int retval;

    buffer64[0] = (BYTE)ST_TOUCH_ERASE_SECTOR;
    buffer64[1] = MSB(StartAddress);
    buffer64[2] = MID(StartAddress);
    buffer64[3] = LSB(StartAddress);

    retval = USBBulkDataWrite(buffer64, 64, 1000);
    return(retval);
}

// erase entire flash NVM
int BulkErase() {
    BYTE buffer64[64];
    int retval;

    buffer64[0] = (BYTE)ST_TOUCH_BULK_ERASE;

    retval = USBBulkDataWrite(buffer64, 64, 1000);
    return(retval);
}

// new worker function to send command table date
static int SendCmdTableData(int command, DWORD TableOffset, unsigned char *Data, int NumBytes) {
    unsigned char outbuffer[64];
    unsigned int retval;
    int i;

    // writing to flash NVM is expensive, don't do if not necessary
    if(UseSTM32 && (command == ST_TOUCH_SET_NV))
    {
        for(i=0; i<NumBytes; i++)
        {
            if(Data[i] != 0xFF)
            {
                break;
            }
        }
        if(i==NumBytes)
        {
            return ST_USB_ERR_NONE;   // nothing to overwrite
        }
    }

    outbuffer[0] = (unsigned char)command;
    outbuffer[1] = MSB(TableOffset);
    outbuffer[2] = MID(TableOffset);
    outbuffer[3] = LSB(TableOffset);
    outbuffer[4] = NumBytes;

    for ( i = 0; i<NumBytes; i++ ) {
        outbuffer[5 + i] = *Data;
        Data++;
    }

    retval = USBBulkDataWrite(outbuffer, 64, 1000);
    return(retval);
}

// new general purpose function
int SendCmdTable(int command, int StartAddress, unsigned char *DataBuff, int TableSize) {
    int retval;

    int blocksize;
    int bytecount;
    int remainingBytes;
    int TableOffset;

    unsigned char *bptr;
    unsigned char *Data = DataBuff;
    unsigned char buffer64[64];
    unsigned char buffer[0x8000];

    // if writing flash NVM, might need to combine new data with what is already there 
    if(UseSTM32 && (command == ST_TOUCH_SET_NV))
    {
        // modifiying calibration data
        if(StartAddress <= STM32_NVM_SECTOR0_END)
        {
            // limit writing to sector 0 (calibration data)
            if(TableSize + StartAddress > STM32_NVM_SECTOR1_START)
            {
                TableSize = STM32_NVM_SECTOR1_START - StartAddress;
            }

            // read contents of sector 0
            buffer64[0] = (BYTE)ST_TOUCH_GET_NV_HUGE;
            buffer64[1] = MSB(STM32_NVM_SECTOR0_START);
            buffer64[2] = MID(STM32_NVM_SECTOR0_START);
            buffer64[3] = LSB(STM32_NVM_SECTOR0_START);
            buffer64[4] = MSB((STM32_NVM_SECTOR1_START - STM32_NVM_SECTOR0_START));
            buffer64[5] = MID((STM32_NVM_SECTOR1_START - STM32_NVM_SECTOR0_START));
            buffer64[6] = LSB((STM32_NVM_SECTOR1_START - STM32_NVM_SECTOR0_START));
            retval = USBBulkDataWrite(buffer64, 64, 1000);
            if(retval == ST_USB_ERR_NONE)
            {
                retval = USBBulkDataRead(buffer, (STM32_NVM_SECTOR1_START - STM32_NVM_SECTOR0_START), 5000);
            }
            if(retval != ST_USB_ERR_NONE)
            {
                return retval;
            }

            // will the new data change what's already there?
            if(memcmp(buffer+StartAddress, Data, TableSize) == 0)
            {
                // no, don't need to write what's already written
                return ST_USB_ERR_NONE;
            }
            // yes, need to update what's there
            memcpy(buffer+StartAddress, Data, TableSize);

            // prepare to write to sector 0
            EraseSector(STM32_NVM_SECTOR0_START);

            StartAddress = STM32_NVM_SECTOR0_START;
            Data = buffer;
            TableSize = (STM32_NVM_SECTOR1_START - STM32_NVM_SECTOR0_START);
        }
        // modifying internal or user data
        else if(StartAddress <= STM32_NVM_SECTOR1_END)
        {
            // limit writing to sector 1 (internal or user data)
            if(TableSize + StartAddress > STM32_NVM_SECTOR2_START)
            {
                TableSize = STM32_NVM_SECTOR2_START - StartAddress;
            }

            // read contents of sector 1
            buffer64[0] = (BYTE)ST_TOUCH_GET_NV_HUGE;
            buffer64[1] = MSB(STM32_NVM_SECTOR1_START);
            buffer64[2] = MID(STM32_NVM_SECTOR1_START);
            buffer64[3] = LSB(STM32_NVM_SECTOR1_START);
            buffer64[4] = MSB((STM32_NVM_SECTOR2_START - STM32_NVM_SECTOR1_START));
            buffer64[5] = MID((STM32_NVM_SECTOR2_START - STM32_NVM_SECTOR1_START));
            buffer64[6] = LSB((STM32_NVM_SECTOR2_START - STM32_NVM_SECTOR1_START));
            retval = USBBulkDataWrite(buffer64, 64, 1000);
            if(retval == ST_USB_ERR_NONE)
            {
                retval = USBBulkDataRead(buffer, (STM32_NVM_SECTOR2_START - STM32_NVM_SECTOR1_START), 5000);
            }
            if(retval != ST_USB_ERR_NONE)
            {
                return retval;
            }

            // will the new data change what's already there?
            if(memcmp(buffer+(StartAddress-STM32_NVM_SECTOR1_START), Data, TableSize) == 0)
            {
                // no, don't need to write what's already written
                return ST_USB_ERR_NONE;
            }
            // yes, need to update what's there
            memcpy(buffer+(StartAddress-STM32_NVM_SECTOR1_START), Data, TableSize);

            // prepare to write to sector 1
            EraseSector(STM32_NVM_SECTOR1_START);

            StartAddress = STM32_NVM_SECTOR1_START;
            Data = buffer;
            TableSize = (STM32_NVM_SECTOR2_START - STM32_NVM_SECTOR1_START);
        }
        // modify EIM gradient data
        else
        {
            // never want to modify EIM gradient, always overwrite
            EraseSector(STM32_NVM_SECTOR2_START);
            EraseSector(STM32_NVM_SECTOR3_START);
            EraseSector(STM32_NVM_SECTOR4_START);
        }
    }
    else if(UseSTM32 && (command == ST_TOUCH_GET_NV_HUGE))
    {
        buffer64[0] = (BYTE)command;
        buffer64[1] = MSB(StartAddress);
        buffer64[2] = MID(StartAddress);
        buffer64[3] = LSB(StartAddress);
        buffer64[4] = MSB(TableSize);
        buffer64[5] = MID(TableSize);
        buffer64[6] = LSB(TableSize);
        retval = USBBulkDataWrite(buffer64, 64, 1000);
        if(retval == ST_USB_ERR_NONE)
        {
            retval = USBBulkDataRead(DataBuff, TableSize, 5000);
        }

        return retval;
    }

    TableOffset = StartAddress;
    remainingBytes = TableSize;
    bytecount = 0;
    bptr = Data;
    retval = ST_USB_ERR_NONE;

    while (remainingBytes>0 && retval == ST_USB_ERR_NONE) {
        blocksize = ( remainingBytes>58 ) ? 58 : remainingBytes;

        retval = SendCmdTableData(command, (DWORD)TableOffset, bptr, blocksize);

        if (retval != ST_USB_ERR_NONE) {
            return(retval);
        }

        remainingBytes -= blocksize;
        TableOffset += blocksize;
        bptr += blocksize;
    }

    // if bit 7 is set, a single 64-byte reply packet is expected.
    if ((command & 0x0080) != 0) {
        // note that "*Data" buffer must be at least 64 bytes in this case...regardless of
        // the reported size in "TableSize"
        retval = USBBulkDataRead(Data, 64, 1000);

        if (retval != ST_USB_ERR_NONE) {
            return(retval);
        }
        if (*Data != (command & 0x7F)) { // check that first byte echoes command minus bit 7
            return ST_USB_ERR_INVALID;    // not the packet we were expecting
        }

    }

    return retval;
}

int GrabImageST_USB(unsigned char *Buffer, unsigned long BufferSize, int grabflag) {
    int retval;

    unsigned char writedata;
    unsigned char bytes64[64];
    unsigned char grabtype;
    unsigned short packetcount;
    unsigned long roundBufferSize;
    unsigned long lastPacketSize;

    lastPacketSize = BufferSize %64;
    roundBufferSize = BufferSize - lastPacketSize; // round down to nearest 64 bytes
    packetcount = (unsigned short)((unsigned long)( roundBufferSize >> 6 ));

    if (lastPacketSize != 0) {
        packetcount++;
    }

    if ((grabflag&GRAB_TYPE_NEW_GRAB) == 0) {          // set up grab the old way for old readers
        grabtype = ST_DATAFLOW;                        // by default...normal grab

        if ((grabflag&GRAB_TYPE_SETTINGS_GRAB) != 0) { // best settings grab
            grabtype = ST_BESTSETT;
        }

        retval = Write_Physical_Reg(grabtype, &writedata, 1); // start flow of image data into buffers
        if (retval != ST_USB_ERR_NONE) {
            return(retval);
        }
    }
    else { // set up grab the new way for new readers
        // ( the new way means telling the reader
        // how many packets to send, so that no
        // extra packet is left dangling at the
        // end of the grab.)

        retval = SendCmdTableData(ST_TOUCH_NEW_GRAB, packetcount, bytes64, /* this is not used in this case */
                                  grabflag & GRAB_TYPE_SETTINGS_GRAB);
    }

    retval = USBBulkDataRead(Buffer, roundBufferSize, 5000);

    if (retval == ST_USB_ERR_NONE) {
        if (lastPacketSize != 0) {                      // if last packet is not 64 bytes
            retval = USBBulkDataRead(bytes64, 64, 500); // grab 1 full 64 byte packet
            if (retval == ST_USB_ERR_NONE) {

                // copy the last bytes onto the end of the buffer.
                memcpy(Buffer + roundBufferSize, bytes64, lastPacketSize);
            }
        }
    }

    return(retval);
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////            The following code are called by PP                  /////////////
/////////////////////////////////////////////////////////////////////////////////////////

int __stdcall OnGetDeviceListUSB(DeviceInfoStruct *p_device_info)
{
    int status = 0;    


    return status;
}

int __stdcall OnSelectUSB(char *createFileName, HANDLE the_event)
{
    int status = 0, data;
    
    status = InitializeST_USB( createFileName, the_event);

    // ST9 fix
    if((status == ST_USB_ERR_NONE) && (OnReadUSB(0, &data) == ST_USB_ERR_USBRD))
    {
        CleanUpST_USB();
        status = InitializeST_USB( createFileName, the_event);
    }

    return status;
}

int __stdcall OnCloseUSB()
{
    int status = 0;    

    status = CleanUpST_USB();

    return status;
}

int __stdcall OnReadUSB(int RegNum, int *DataI2C)
{
    int status = 0;

    status = Read_Physical_Reg(RegNum, DataI2C);
    
    return status;
}

int __stdcall OnWriteUSB(int RegNum, int DataI2C)
{
    int status = 0;
    unsigned char bytedata;

    bytedata = (unsigned char) DataI2C;
    status = Write_Physical_Reg( RegNum, &bytedata, 1 );

    return status;
}

int __stdcall OnGrabUSB(unsigned char *Buffer, unsigned long BufferSize, int imagetype)
{
    int status = 0;

    status = GrabImageST_USB( Buffer, BufferSize, imagetype);

    return status;
}

int __stdcall OnSendCmdTableUSB( int command, int StartAddress,    unsigned char * bptr, int TableSize)
{
    int status = 0;

    status = SendCmdTable( command, StartAddress, bptr, TableSize );

    return status;
}    

int __stdcall OnSendSettingsUSB( unsigned char * bptr, int NumSetts)
{
    int status = 0;

    status = SendCmdTable(ST_TOUCH_LOAD_TABLE, 0x2000, bptr, NumSetts*3);

    return status;
}


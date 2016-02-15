/**
 * @file query.c
 * @brief Version 1.0 Function supported by Rev. E sensor
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#include <time.h>
#include "perfectprintint.h"
#include "ioreg.h"
#include "tci.h"
#include "qualeng.h"
#include "badlines_new.h"
#include "touchip.h"


//--------------------------- Local Functions -------------------------------
static int STiResetDevice();
static int STiWriteAppReg(int Data);
static int STiReadAppReg(int *Data);


//-------------------------- Function definitions ---------------------------

int STGetTCIRevision(char *i_pTCIRev, char *i_szDesc, int DescBufSize)
{
    char s[255] = REVISION;
    UINT i;

    // check for NULL parameters
    if((i_pTCIRev == NULL) || (i_szDesc == NULL))
        return STERR_BADPARAMETER;

    // validate description buffer size
    if((UINT)DescBufSize < (strlen(REVISION)+3))
    {
        return STERR_BUFFER_TOO_SMALL;
    }

    // load description into buffer
    for(i=0; i<strlen(REVISION); i++)
    {
        i_szDesc[i] = s[i];
    }

    // special suffixes telling which conditional feature flags are set (defined)
    // if none, last char of string will be blank
    // intent is to not have any suffix chars for what is released to customer
    i_szDesc[i++] = ' ';

    // insert here special suffix
    if(!COLUMN_PRECHARGE)
    {
        i_szDesc[i++] = 'p';
    }
    i_szDesc[i] = '\0';

    // load release revision
    i_pTCIRev[0] = RELEASE;
    i_pTCIRev[1] = VERSION;
    i_pTCIRev[2] = VERSION_SUB;
    i_pTCIRev[3] = VERSION_SUB_SUB;

    return STERR_OK;
}

int STResetDevice()
// External variation, with error returns only as in API spec
{
    int iResult = STERR_OK;

    // call internal variation
    iResult = STiResetDevice();

    // errors must be visible but not known by the user
    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) &&
       (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        iResult -= 100;
    }

    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STResetDevice, with unfiltered error returns
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return @ref STERR_SENSOR_NOT_CALIBRATED if sensor needs to be calibrated first
 * @return (other values) Errors from functions called internally
 */
//---------------------------------------------------------------------------
int STiResetDevice()
// Internal variation, with unfiltered error returns
{
    int iResult = STERR_OK;

    // check if STOpen() has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    // set device to imaging mode
    iResult = STSetChipState(IMAGING_MODE);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    // initialize sensor registers
    iResult = InitSensorRegisters(Glbv.Pub.SensorVersion);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    // set default setting
    if((Glbv.Prv.rp->ReaderMode == READER_MODE_STD) && (Glbv.Prv.rp->ReaderType != READER_TYPE_STM32))
    {
        iResult = GetSelSettings(DEF_STD_SETTING);
    }
    else
    {
        iResult = GetSelSettings(DEF_EIM_SETTING);
    }
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    // write EIM offset
    if(Glbv.Prv.rp->ReaderMode != READER_MODE_STD)
    {
        if(Glbv.Prv.rp->EIMOffset != 0)
        {
            WriteLogicalRegister(OFFSET, Glbv.Prv.rp->EIMOffset, Force);
        }
        else
        {
            return STERR_SENSOR_NOT_CALIBRATED;
        }
    }

    // set full-sized window
    iResult = SetWindowFullSize();
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    return STERR_OK;
}

int STWriteAppReg(int Data)
// External variation, with error returns only as in API spec
{
    int iResult = STERR_OK;

    // call internal variation
    iResult = STiWriteAppReg(Data);

    // errors must be visible but not known by the user
    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) &&
       (iResult!=STERR_BAD_DEVICE) && (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        iResult -= 100;
    }

    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STWriteAppReg, with unfiltered error returns
 *
 * @param Data  The value to be written
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 * @return @ref STERR_BAD_DEVICE if no device has been defined/open
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return (other values) Errors from functions called internally
 */
//---------------------------------------------------------------------------
int STiWriteAppReg(int Data)
// Internal variation, with unfiltered error returns
{
    int iResult = STERR_OK;

    // check if STOpen() has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    iResult = WriteLogicalRegister(APP_REG, Data, Force);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    return iResult;
}

int STReadAppReg(int *Data)
// External variation, with error returns only as in API spec
{
    int iResult = STERR_OK;

    // call internal variation
    iResult=STiReadAppReg(Data);

    // errors must be visible but not known by the user
    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_BADPARAMETER) &&
       (iResult!=STERR_BAD_DEVICE) && (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        iResult -= 100;
    }

    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STReadAppReg, with unfiltered error returns
 *
 * @param Data  Address of an integer variable to save the value read.
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_BAD_DEVICE if no device has been defined/open
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return (other values) Errors from functions called internally
 */
//---------------------------------------------------------------------------
int STiReadAppReg(int *Data)
{
    int iResult = STERR_OK;

    // check if STOpen() has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    // check for NULL parameters
    if(Data == NULL)
    {
        return STERR_BADPARAMETER;
    }

    iResult = ReadLogicalRegister(APP_REG, Data, Force);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    return iResult;
}

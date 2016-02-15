/**
 * @file tci_ext.c
 * Version 1.0 Function supported by Rev. E sensor
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#include <time.h>
#include "perfectprintint.h"
#include "badlines_new.h"
#include "touchip.h"
#include "qualeng.h"
#include "ioreg.h"
#include "tci.h"
#include "nvm.h"
#include "orientation.h"

//--------------------------------- Defines ---------------------------------
#define TC_IDD_OFF_NOW 255      ///< turns off TC power


//--------------------------------- Macros ----------------------------------
/**
 * @defgroup ansi381macros ANSI 381 Macros
 * Macros for ANSI 381 calculation
 * @{
 */
#define BF_GETBYTE0(x) ((BYTE)((x & 0xFF)))
#define BF_GETBYTE1(x) ((BYTE)((x & 0xFF00) >> 8))
#define BF_GETBYTE2(x) ((BYTE)((x & 0xFF0000) >> 16))
#define BF_GETBYTE3(x) ((BYTE)((x & 0xFF000000) >> 24))
/** @} */


//----------------------------- Local Variables -----------------------------
/**
 * @defgroup fingeventhndlrs Finger Event Handlers
 * Finger present/left event handlers
 * @{
 */
HANDLE hmyFingerPresent;
HANDLE hmyFingerLeft;
/** @} */

//--------------------------- Local Functions -------------------------------
static int STiInitialize(HANDLE hFingerPresent, HANDLE hFingerLeft);
static int STiTerminate();
static int STiAuthentify(unsigned char *AlgoId, unsigned char *PassPhrase);
static int STiOpen(SensorDescr *SensorInfo);
static int STiClose();
static int STiGrab(unsigned char *Buffer, unsigned long BufferSize);
static int STiGrabBestSett(unsigned char *Buffer, unsigned long BufferSize);
static int STiGrabSelSett(unsigned char *Buffer, unsigned long BufferSize, int SelSett);
static int STiGrabMosaic(unsigned char *Buffer, unsigned long BufferSize);
static int STiImage508DPITo500DPI(unsigned char *inBuffer, int inWidth, int inHeight, unsigned char *outBuffer, int *outWidth, int *outHeight);
static int STiImageToANSI381(unsigned char *inBuffer, int inWidth, int inHeight, unsigned char *outBuffer, unsigned long outBufferSize);
static int STiGetBestSett();
static int STiInitSensorPar(int data);
static int STiSetChipState(int data);
static int STiGetEntropy(void *apEntropy, int *apnSize);
static int AdjustImage(unsigned char *Buffer, unsigned long BufferSize);
static int AdjustImage_EIMCompensation(unsigned char *Buffer, unsigned long BufferSize);
static int AdjustImage_FingerQuality(unsigned char *Buffer, unsigned long BufferSize);
static int AdjustImage_Orientation(unsigned char *Buffer, unsigned long BufferSize, WindowSettings *pLogWinSett);


//-------------------------- Function definitions ---------------------------

//---------------------------------------------------------------------------
int STInitialize(HANDLE hFingerPresent, HANDLE hFingerLeft)
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STInitialize -----------\n");
#endif

    iResult = STiInitialize(hFingerPresent, hFingerLeft);

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_DEVICEINUSE) && 
       (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        // errors must be visible but not known by the user
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STInitialize, with unfiltered error returns
 *
 * hFingerLeft is not used in version 1.1 and earlier it's just 
 * for future compatibility.  It should be set NULL in version 1.1 
 * and earlier
 *
 * @param hFingerPresent For future compatibility.  Use @c NULL.
 * @param hFingerLeft    For future compatibility.  Use @c NULL.
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_DEVICEINUSE if another device is in use
 * @return @ref STERR_BAD_STATE if chip in bad state
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error 
 * @return @ref STERR_SENSOR_VERSION if sensor version invalid
 * commmunicating with the sensor
 * @return @ref STERR_MEMORY_MALLOC_FAIL if memory allocation failure
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
int STiInitialize(HANDLE hFingerPresent, HANDLE hFingerLeft)
// Internal variation, with unfiltered error returns
{
    int iResult=STERR_OK;
    int busaddr;

    if((Glbv.Prv.PP_IntStatus!=NotInUse) && (Glbv.Prv.PP_IntStatus!=PP_Terminate))
    {
        return STERR_DEVICEINUSE;
    }

    // initialize globals
    InitGlobalVar();

    // create event handler for interrupt
    Glbv.Prv.hFingerEvent = hFingerPresent;

    // intialize and check USB driver
    if(InitializeUSB() != STERR_USB_ERR_NONE)
    {
        Glbv.Pub.DeviceType = (BYTE)ST_NO_DEVICE;

        STTerminate();

        return STERR_NODEVICE;
    }
    else
    {
        Glbv.Pub.DeviceType = (BYTE)ST_USB;
    }

///////////////////////////////////////////////////////////
// Init the TC IDD current limit to ~100 ma.
///////////////////////////////////////////////////////////
    iResult = WriteLogicalRegister(TC_IDD, 205, Force);
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }

///////////////////////////////////////////////////////////
// Determine Firmware version
///////////////////////////////////////////////////////////
    iResult = TCI_CheckFirmware();
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }

///////////////////////////////////////////////////////////
// Init the cached NVM directory
///////////////////////////////////////////////////////////
    iResult = InitNVMDir(&Glbv.Prv.nvmdir);
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }

    // reload the internal register table
    iResult = LoadRegTable();
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }

///////////////////////////////////////////////////////////
// Determine which sensor version is attached
///////////////////////////////////////////////////////////
    iResult = TCI_CheckSensorVersion();
    if(iResult == STERR_SENSOR_VERSION)
    {
        // in case of partial latch-up turn off TC entirely
        WriteLogicalRegister(TC_IDD, TC_IDD_OFF_NOW, Force);

        // give the TC power plenty of time to discharge
        Sleep(100);

        STTerminate();
        return iResult;
    }

///////////////////////////////////////////////////////////
// Load the reader specific variables
///////////////////////////////////////////////////////////
    LoadReaderProfile(Glbv.Prv.rp, FALSE);

///////////////////////////////////////////////////////////
// Init the sensor-dependent behavior
///////////////////////////////////////////////////////////
    InitConfigFlag(Glbv.Pub.SensorVersion, &Glbv.Prv.ConfigFlag);

///////////////////////////////////////////////////////////
// Init the settings table and calibration settings
///////////////////////////////////////////////////////////
    InitSettingsTable(Glbv.Pub.SensorVersion);

///////////////////////////////////////////////////////////
// Initialize window settings
///////////////////////////////////////////////////////////
    iResult = InitWindowSize(Glbv.Pub.SensorVersion);
    if(iResult != STERR_OK)
    {
        return iResult;
    }

///////////////////////////////////////////////////////////
// Change any globals that can be configured from the NVM
// ...this may overwrite the settings table above with one
// loaded from the NVM config data, as well as window size,
// sensor version, etc.
///////////////////////////////////////////////////////////
    iResult = OverrideGlobals();
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }

///////////////////////////////////////////////////////////
// Here we are overriding the overriden global values to 
// make sure that the EIM values are used for EIM readers. 
// It is highly possible for a user to run an application 
// such as TcTest or ReaderTest[X] that does not take EIM 
// readers into account and accidentally override the EIM 
// values with standard ones.  This extra override will 
// prevent that from happening - a safeguard.
///////////////////////////////////////////////////////////
    if((Glbv.Prv.rp->ReaderMode == READER_MODE_EIM) || (Glbv.Prv.rp->ReaderMode == READER_MODE_EIM_LITE))
    {
        Glbv.Prv.PeriodWidth = 19;
        Glbv.Prv.ChargeWidth = 11;
        Glbv.Prv.ResetWidth  = 10;
    }

///////////////////////////////////////////////////////////
// Send the setting table to the reader
///////////////////////////////////////////////////////////
    iResult = DownloadSettingTable(Glbv.Prv.TCSettingTable, Glbv.Prv.TotalSettings);
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }

///////////////////////////////////////////////////////////
// Set the Chip in active mode
///////////////////////////////////////////////////////////
    iResult = STiSetChipState(IMAGING_MODE);
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }

    // init the sensor depending on the version
    iResult = InitSensorRegisters(Glbv.Pub.SensorVersion);
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }

///////////////////////////////////////////////////////////
// Set up the TC IDD current limit
///////////////////////////////////////////////////////////
    iResult = TCI_GetIDD(&Glbv.Prv.IDD_Init);
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }

    iResult = TCI_SetIDDLimit(Glbv.Prv.IDD_Init);
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }

    // check if the flag to override shorted column detection has been set
    if((Glbv.Prv.TC_OverRideFlags&TC_SC_DETECT_OVERRIDE) == 0)
    {
        // writing to the SC_DET_REP register forces the firmware to detect 
        // and repair shorted columns. This may reduce the TC_IDD.
        iResult = WriteLogicalRegister(SC_DET_REP, SCDET_TH128_OFF63, Force);
        if(iResult != STERR_OK)
        {
            STTerminate();
            return iResult;
        }
    }

    // set window full size
    iResult = SetWindowFullSize();
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }

    // map the event handler
    hmyFingerPresent = hFingerPresent;
    // NOT USED IN VERSION 1.1
    hmyFingerLeft = hFingerLeft;

    // enable the Chip Interrupt
    iResult = WriteLogicalRegister(DETPD, 1, NoForce);
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }
    iResult = WriteLogicalRegister(NRSTAS, 1, NoForce);
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }
    iResult = WriteLogicalRegister(NRSTAS, 0, NoForce);
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }
    iResult = WriteLogicalRegister(NRSTAS, 1, Force);
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }

    iResult = ReadLogicalRegister(USB_BUSADDR, &busaddr, Force);
    if(iResult != STERR_OK)
    {
        STTerminate();
        return iResult;
    }
    Glbv.Pub.USB_BusAddress = (BYTE)busaddr;

    // initialize the table for STAuthentify with algorithm ids and passphrases
    InitAuthentify();

#ifdef _DEBUG
    printf("STInitialize returned %d\n",STERR_OK);
#endif

    // update status variable
    Glbv.Prv.PP_IntStatus = PP_Initialize;

    return STERR_OK;
}

//---------------------------------------------------------------------------
int STOpen(SensorDescr *SensorInfo)
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STOpen -----------\n");
#endif

    iResult = STiOpen(SensorInfo);

    if((iResult!=STERR_OK) && (iResult!=STERR_NOTAUTHORIZED) && (iResult!=STERR_NODEVICE) && 
       (iResult!=STERR_DEVICEINUSE) && (iResult!=STERR_BADPARAMETER) && (iResult!=STERR_NOT_SUPPORTED) &&
       (iResult!=STERR_SENSOR_COMMUNICATION) && (iResult!=STERR_SENSOR_NOT_CALIBRATED) && 
       (iResult!=STERR_BAD_GRADIENT_DATA) && (iResult!=STERR_BEZEL_NOT_CONNECTED))
    {
        // errors must be visible but not known by the user
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STOpen, with unfiltered error returns
 *
 * @param SensorInfo Information about the sensor opened (returned)
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NOTAUTHORIZED if sensor has not been authentified
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_DEVICEINUSE if another device is in use
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_NOT_SUPPORTED if device is not supported
 * @return @ref STERR_BAD_STATE if chip in bad state
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return @ref STERR_MEMORY_MALLOC_FAIL if memory allocation failure
 * @return @ref STERR_WIN_REG_FAIL if there is a Windows registry failure
 * @return @ref STERR_FILE_IN_OUT_FAIL if there is a file I/O failure
 * @return @ref STERR_SENSOR_NOT_CALIBRATED if sensor needs to be calibrated first
 * @return @ref STERR_BAD_GRADIENT_DATA if gradient data in NVM, Windows registry or 
 * external file is missing/corrupt
 * @return @ref STERR_BEZEL_NOT_CONNECTED if the bezel is not connected
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
int STiOpen(SensorDescr *SensorInfo) 
// Internal variation, with unfiltered error returns
{
    SensorDescr PhySensorDat;
    int iResult=STERR_OK;
    int data=0;

    if((Glbv.Prv.PP_IntStatus==NotInUse) || (Glbv.Prv.PP_IntStatus==PP_Terminate))
    {
        return STERR_NODEVICE;
    }

    if(Glbv.Prv.PP_IntStatus == PP_Open)
    {
        return STERR_DEVICEINUSE;
    }

    // check if it's authorized (status = PP_Close is valid because it means 
    // that the user had already open the sensor, and so also authentify)
    if((Glbv.Prv.PP_IntStatus!=PP_Authentify) && (Glbv.Prv.PP_IntStatus!=PP_Close))
    {
        return STERR_NOTAUTHORIZED;
    }

    // check for NULL parameters
    if(SensorInfo == NULL)
    {
        return STERR_BADPARAMETER;
    }

    // check for ESD event
    iResult = Check_ESD_CommunicationError();
    if(iResult != STERR_OK)
    {
        return iResult;
    }

    // bezel connection test
    if((Glbv.Prv.TC_OverRideFlags&TC_DO_BEZEL_CONNECTION_TEST) != 0)
    {
        if(Glbv.Prv.rp->ReaderType==READER_TYPE_STM32)
        {
            if((Glbv.Prv.rp->ReaderMode == READER_MODE_EIM) || (Glbv.Prv.rp->ReaderMode == READER_MODE_EIM_LITE))
            {
                iResult = ReadLogicalRegister(MISC_CNTL, &data, Force);
                if(iResult != STERR_OK)
                {
                    return iResult;
                }

                if(!(data & 0x40))
                {
                    return STERR_BEZEL_NOT_CONNECTED;
                }
            }
        }
    }

    // initialize globals for bad line repair
    InitializeRepairGlobals();

    // read reader profile structure from NVM
    iResult = LoadReaderProfile(Glbv.Prv.rp, TRUE);
    if(iResult != STERR_OK)
    {
        return iResult;
    }


    // set IDD limit back to init value (otherwise the next reset, 
    // which clears any repairs, may trigger an overcurrent event.)
    iResult = TCI_SetIDDLimit(Glbv.Prv.IDD_Init);
    if(iResult != STERR_OK)
    {
        return iResult;
    }

    // reset chip and set in default mode
    iResult = Reset();
    if(iResult != STERR_OK)
    {
        return iResult;
    }

    // deploy the repair data
    iResult = SensorRepair();
    if((iResult!=STERR_OK) && (iResult!=STERR_READ_REPAIR_INFO))
    {
        return iResult;
    }

    // check if the flag to override shorted column detection has been set
    if((Glbv.Prv.TC_OverRideFlags&TC_SC_DETECT_OVERRIDE) == 0)
    {
        // writing to the SC_DET_REP register forces the firmware to detect 
        // and repair shorted columns. This may reduce the TC_IDD.
        iResult = WriteLogicalRegister(SC_DET_REP, SCDET_TH128_OFF63, Force);
        if(iResult != STERR_OK)
        {
            return iResult;
        }
    }

    // now set the TC_IDD limit after repairs
    iResult = TCI_GetIDD(&Glbv.Prv.IDD_Dynamic);
    if(iResult != STERR_OK)
    {
        return iResult;
    }
    iResult = TCI_SetIDDLimit(Glbv.Prv.IDD_Dynamic);
    if(iResult != STERR_OK)
    {
        return iResult;
    }

    // load the output data
    PhySensorDat.SensorVersion = Glbv.Pub.SensorVersion;
    PhySensorDat.ImageWidth    = GlbWin->MaxWidth;
    PhySensorDat.ImageHeight   = GlbWin->MaxHeight;
    iResult = MapPhysicalToLogicalSensorData(SensorInfo, &PhySensorDat);
    if(iResult != STERR_OK)
    {
        return iResult;
    }

    // set initial default settings
    if((Glbv.Prv.rp->ReaderMode == READER_MODE_STD) && (Glbv.Prv.rp->ReaderType != READER_TYPE_STM32))
    {
        Glbv.Prv.CurrentSetting = DEF_STD_SETTING;
    }
    else
    {
        Glbv.Prv.CurrentSetting = DEF_EIM_SETTING;
    }
    iResult = GetSelSettings(Glbv.Prv.CurrentSetting);
    if(iResult != STERR_OK)
    {
        return iResult;
    }

    // if in EIM, make sure the sensor has been calibrated
    if(Glbv.Prv.rp->ReaderMode != READER_MODE_STD)
    {
        //sensor calibrated?
        if(Glbv.Prv.rp->EIMOffset != 0)
        {
            iResult = WriteLogicalRegister(OFFSET, Glbv.Prv.rp->EIMOffset, Force);
        }
        else
        {
            return STERR_SENSOR_NOT_CALIBRATED;
        }
    }

    Glbv.Prv.PP_IntStatus = PP_Open;

    return iResult;
}

//---------------------------------------------------------------------------
int STTerminate()
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STTerminate-----------\n");
#endif

    iResult = STiTerminate();

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE))
    {
        // errors must be visible but not known by the user
        //iResult = STERR_GENERAL_ERROR;
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STTerminate, with unfiltered error returns
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
int STiTerminate()
// Internal variation, with unfiltered error returns
{
    int iResult=STERR_OK;

    if(Glbv.Prv.PP_IntStatus == PP_Open)
    {
        STClose();
    }

    switch(Glbv.Pub.DeviceType)
    {
    case ST_NO_DEVICE:
        Glbv.Pub.DeviceType = (BYTE)ST_NO_DEVICE;
        Glbv.Prv.PP_IntStatus = PP_Terminate;
        return STERR_NODEVICE;
    case ST_USB:
        iResult = WriteLogicalRegister(USB_BUSADDR, 1, Force);    // Un_Authentify
        iResult = STiSetChipState(SLEEP_MODE);
        iResult = OnCloseUSB();
        break;
    default:
        // device not suported
        iResult = STERR_NODEVICE;
        break;
    }

#ifdef _DEBUG
    printf("STTerminate returned %d\n", STERR_OK);
#endif

    Glbv.Prv.PP_IntStatus = PP_Terminate;
    Glbv.Pub.DeviceType = (BYTE)ST_NO_DEVICE;

    return STERR_OK;
}

//---------------------------------------------------------------------------
int STClose()
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STClose -----------\n");
#endif

    iResult = STiClose();

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        // errors must be visible but not known by the user
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STClose, with unfiltered error returns
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
int STiClose()
// Internal variation, with unfiltered error returns
{
    // check if STOpen has terminated
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    // update status varialble
    Glbv.Prv.PP_IntStatus = PP_Close;

    return STERR_OK;
}

//---------------------------------------------------------------------------
int STAuthentify(unsigned char *AlgoID, unsigned char *PassPhrase)
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STAuthentify -----------\n");
#endif

    iResult = STiAuthentify(AlgoID, PassPhrase);

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_DEVICEINUSE) && 
       (iResult!=STERR_BADPARAMETER) && (iResult!=STERR_SENSOR_COMMUNICATION) && 
       (iResult!=STERR_INVALID_LICENSE))
    {
        // errors must be visible but not known by the user
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STAuthentify, with unfiltered error returns
 *
 * @param AlgoId     Algoid string to be checked
 * @param PassPhrase Pass phrase to be returned
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_DEVICEINUSE if another device is in use
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return @ref STERR_INVALID_LICENSE if license is invalid
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
int STiAuthentify(unsigned char *AlgoId, unsigned char *PassPhrase)
// Internal variation, with unfiltered error returns
{
    int iResult;

    // check if STOpen has succesfully completed
    if((Glbv.Prv.PP_IntStatus==NotInUse) || (Glbv.Prv.PP_IntStatus==PP_Terminate))
    {
        return STERR_NODEVICE;
    }

    if(Glbv.Prv.PP_IntStatus == PP_Open)
    {
        return STERR_DEVICEINUSE;
    }

    // check for NULL parameters
    if((AlgoId==NULL) || (PassPhrase==NULL))
    {
        return STERR_BADPARAMETER;
    }

    // Check for signature id
    iResult = CheckAlgoId((char *)AlgoId,(char *)PassPhrase);

#ifdef _DEBUG
    printf("STAuthentify returned %d\n", iResult);
#endif

    return iResult;
}

//---------------------------------------------------------------------------
int STSetWindow(int X0, int Y0, int X, int Y, int IncX, int IncY)
{
    int iResult;
    PhyLogWinSetting logWinSetting;
    PhyLogWinSetting physWinSetting;

#ifdef _DEBUG
    printf("\n----------- Entering STSetWindow -----------\n");
#endif

    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    logWinSetting.X0    = X0;
    logWinSetting.Y0    = Y0;
    logWinSetting.X     = X;
    logWinSetting.Y     = Y;
    logWinSetting.IncX  = IncX;
    logWinSetting.IncY  = IncY;

    iResult = MapLogicalToPhysicalWindow(&physWinSetting, &logWinSetting);
    if(iResult != STERR_OK)
    {
        return iResult;
    }

    iResult = SetWindow(physWinSetting.X0, physWinSetting.Y0, physWinSetting.X, physWinSetting.Y, physWinSetting.IncX, physWinSetting.IncY);

    // make sure return value is as published in API spec
    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_BADPARAMETER) && 
       (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        // errors must be visible but not known by the user
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
int STGrab(unsigned char *Buffer, unsigned long BufferSize)
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STGrab -----------\n");
#endif

    iResult = STiGrab(Buffer, BufferSize);

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_BAD_STATE) && 
       (iResult!=STERR_BADPARAMETER) && (iResult!=STERR_NO_FINGER) && 
       (iResult!=STERR_BAD_QUALITY_FINGER) && (iResult!=STERR_FAINT_FINGER) && 
       (iResult!=STERR_LATENT_FINGERPRINT) && (iResult!=STERR_BUFFER_TOO_SMALL) && 
       (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        // errors must be visible but not known by the user
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
int STGrabNFD(unsigned char *Buffer, unsigned long BufferSize)
// External variation, with error returns only as in API spec
{
    int iResult, tempflags;

#ifdef _DEBUG
    printf("\n----------- Entering STGrabNFD -----------\n");
#endif

    // save state of override flags
    tempflags = Glbv.Prv.TC_OverRideFlags;

    // override finger detect
    Glbv.Prv.TC_OverRideFlags |= TC_FINGER_OVERRIDE;

    iResult = STiGrab(Buffer, BufferSize);

    // restore flags
    Glbv.Prv.TC_OverRideFlags = tempflags;

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_BAD_STATE) && 
       (iResult!=STERR_BADPARAMETER) && (iResult!=STERR_BUFFER_TOO_SMALL) && 
       (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        // errors must be visible but not known by the user
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
int STGrabBestSettNFD(unsigned char *buf, unsigned long bufsize)
// External variation, with error returns only as in API spec
{
    int iResult, tempflags;

#ifdef _DEBUG
    printf("\n----------- Entering STGrabBestSettNFD -----------\n");
#endif

    // save state of override flags
    tempflags = Glbv.Prv.TC_OverRideFlags;

    // override finger detect
    Glbv.Prv.TC_OverRideFlags |= TC_FINGER_OVERRIDE;

    iResult = STiGrabBestSett(buf, bufsize);

    // restore flags
    Glbv.Prv.TC_OverRideFlags = tempflags;

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_BAD_STATE) && 
       (iResult!=STERR_BADPARAMETER) && (iResult!=STERR_SENSOR_COMMUNICATION) && 
       (iResult!=STERR_BUFFER_TOO_SMALL))
    {
        // errors must be visible but not known by the user
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
int STGrabQuick(unsigned char *Buffer, unsigned long BufferSize)
// External variation, with error returns only as in API spec
{
    int iResult, tempflags;

#ifdef _DEBUG
    printf("\n----------- Entering STGrabFD83 -----------\n");
#endif

    // save state of override flags
    tempflags = Glbv.Prv.TC_OverRideFlags;

    // override finger detect and OTF repair
    Glbv.Prv.TC_OverRideFlags |= (TC_FINGER_OVERRIDE|TC_OTF_OVERRIDE);

    iResult = STiGrab(Buffer, BufferSize);

    // restore flags
    Glbv.Prv.TC_OverRideFlags = tempflags;

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_BAD_STATE) && 
        (iResult!=STERR_BADPARAMETER) && (iResult!=STERR_BUFFER_TOO_SMALL) && 
        (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        // errors must be visible but not known by the user
        //iResult = STERR_GENERAL_ERROR;
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @brief Make adjustments on grabbed image
 *
 * @param Buffer      Buffer in which is saved the grabbed image.  Must be 
 *                    pre-allocated in memory.
 * @param BufferSize  The size of the image buffer
 * @param pAlCtx      The context information for the image's antilatent session
 * @param pAlMask     The antilatent image enhancement mask
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_NO_FINGER if no finger detected on sensor
 * @return @ref STERR_FAINT_FINGER if a faint finger is detected on sensor
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return @ref STERR_MEMORY_MALLOC_FAIL if memory allocation failure
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
static int AdjustImage(unsigned char *Buffer, unsigned long BufferSize)
{
    int i, iResult = STERR_OK;
    unsigned int imgWidth = GlbWin->Width;
    unsigned int imgHeight = GlbWin->Height;
    WindowSettings winSett;

    // determine which reader mode is in use by the device
    if(Glbv.Prv.rp->ReaderMode==READER_MODE_STD)
    {
        if(Glbv.Prv.rp->ReaderType == READER_TYPE_STM32)
        {
            // apply EIM compensation to image
            if((Glbv.Prv.TC_OverRideFlags&TC_EIMGRADIENT_OVERRIDE)==0)
            {
                iResult = AdjustImage_EIMCompensation(Buffer, BufferSize);
                if(iResult != STERR_OK)
                {
                    goto Done;
                }
            }

            // enhanced digital gain
            DigitalGainCompensate(Buffer, BufferSize);
        }

        if((Glbv.Prv.TC_OverRideFlags&TC_DIG_GAIN_OVERRIDE)==0)
        {
            // adjust for steelcoat sensors in standard mode
            if(Glbv.Prv.rp->SensorType == SENSOR_TYPE_C_STEELCOAT)
            {
                for(i=0; i<(int)BufferSize; i++)
                {
                    Buffer[i] = Glbv.Prv.FFD_gain[Buffer[i]];
                }
            }
        }
    }
    else
    {
        // apply EIM compensation to image
        if((Glbv.Prv.TC_OverRideFlags&TC_EIMGRADIENT_OVERRIDE)==0)
        {
            iResult = AdjustImage_EIMCompensation(Buffer, BufferSize);
            if(iResult != STERR_OK)
            {
                goto Done;
            }
        }

        // apply finger quality detection and repair algorithm on the image
        if(((Glbv.Prv.TC_OverRideFlags&TC_FINGER_OVERRIDE)==0) && ((Glbv.Prv.TC_OverRideFlags&TC_FINGER_TYPE_OVERRIDE)==0))
        {
            iResult = AdjustImage_FingerQuality(Buffer, BufferSize);
            if(iResult != STERR_OK)
            {
                goto Done;
            }
        }
        else if((Glbv.Prv.TC_OverRideFlags&TC_DIG_GAIN_OVERRIDE)==0)
        {
            // adjust for steelcoat sensors
            if(Glbv.Prv.rp->SensorType == SENSOR_TYPE_C_STEELCOAT)
            {
                for(i=0; i<(int)BufferSize; i++)
                {
                    Buffer[i] = Glbv.Prv.FFD_gain[Buffer[i]];
                }
            }
        }
    }

    // perform orientation if required
    if((Glbv.Prv.Orientation != ORIENT_ZERO) && ((Glbv.Prv.TC_OverRideFlags & TC_ORIENTATION_OVERRIDE) == 0))
    {
        iResult = AdjustImage_Orientation(Buffer, BufferSize, &winSett);
        if(iResult != STERR_OK)
        {
            goto Done;
        }
        imgWidth = winSett.Width;
        imgHeight = winSett.Height;
    }

Done:
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @brief Apply EIM gradient compensation to the image.
 *
 * @param Buffer      Buffer in which is saved the grabbed image.  Must be 
 *                    pre-allocated in memory.  Must be in physical orientation.
 * @param BufferSize  The size of the image buffer
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
static int AdjustImage_EIMCompensation(unsigned char *Buffer, unsigned long BufferSize)
{
    int iResult, i, sum;
    int startCol, stopCol, startRow, startCol_msb, stopCol_msb, startRow_msb;
    int goffsetRow, goffsetCol;

    // get window size
    iResult = ReadLogicalRegister(STARTCOL8, &startCol_msb, Force);
    if(iResult != STERR_OK)
    {
        return iResult;
    }
    iResult = ReadLogicalRegister(STOPCOL8, &stopCol_msb, NoForce);
    if(iResult != STERR_OK)
    {
        return iResult;
    }
    iResult = ReadLogicalRegister(STARTROW8, &startRow_msb, NoForce);
    if(iResult != STERR_OK)
    {
        return iResult;
    }
    iResult = ReadLogicalRegister(STARTCOL, &startCol, NoForce);
    if(iResult != STERR_OK)
    {
        return iResult;
    }
    iResult = ReadLogicalRegister(STOPCOL, &stopCol, NoForce);
    if(iResult != STERR_OK)
    {
        return iResult;
    }
    iResult = ReadLogicalRegister(STARTROW, &startRow, NoForce);
    if(iResult != STERR_OK)
    {
        return iResult;
    }
    startCol += ((startCol_msb == 1)?(256):(0));
    stopCol  += ((stopCol_msb == 1)?(256):(0));
    startRow += ((startRow_msb == 1)?(256):(0));

    // apply gradient to each square of the image grabbed
    goffsetRow = startRow;
    goffsetCol = startCol;
    for(i=0; i<(int)BufferSize; i++)
    {
        // apply gradient (full for EIM, average for EIM-Lite and enhanced standard)
        if(Glbv.Prv.rp->ReaderMode == READER_MODE_EIM)
        {
            sum = (Buffer[i] + Glbv.Prv.rp->EIMGrad[(goffsetRow*GlbWin->MaxWidth)+goffsetCol]);
        }
        else
        {
            sum = (Buffer[i] + Glbv.Prv.rp->EIMGradRowAverage[goffsetRow]);
        }
        if(sum < 1) sum = 1;
        if(sum > 253) sum = 253;
        Buffer[i] = (unsigned char)sum;

        // goto next row?
        if(goffsetCol == stopCol)
        {
            goffsetRow++;
            goffsetCol = startCol;
        }
        else
        {
            goffsetCol++;
        }
    }

    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @brief Apply finger quality detection and repair algorithm on the image
 *
 * @param Buffer      Buffer in which is saved the grabbed image.  Must be 
 *                    pre-allocated in memory.  Must be in physical orientation.
 * @param BufferSize  The size of the image buffer
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NO_FINGER if no finger detected on sensor
 * @return @ref STERR_FAINT_FINGER if finger is faint
 */
//---------------------------------------------------------------------------
static int AdjustImage_FingerQuality(unsigned char *Buffer, unsigned long BufferSize)
{
    int i, iResult, fingContrast = 0;
    BYTE Dynamic_FFD_gain[256];

    // determine the finger type after gradient compensation (faint, normal, etc...)
    iResult = DetermineFingerType(Buffer, GlbWin->Width, GlbWin->Height, &fingContrast);

    // add digital gain to faint finger when desired
    if(iResult==STERR_NO_FINGER)
    {
        memset(Buffer, 0x50, BufferSize);
    }
    else if(((Glbv.Prv.TC_OverRideFlags&TC_DIG_GAIN_OVERRIDE)==0) && 
            ((iResult==STERR_FAINT_FINGER) || (Glbv.Prv.rp->SensorType==SENSOR_TYPE_C_STEELCOAT)))
    {
        // static or dynamic digital gain?
        if(Glbv.Prv.UseDynamicDigitalGainForFaintFinger)
        {
            // dynamic digital gain
            if(Glbv.Prv.rp->SensorType==SENSOR_TYPE_C_STEELCOAT)
            {
                InitDigitalGainTable((BYTE)((fingContrast == 0)?(GAIN_TWO):((BYTE)(((Glbv.Prv.DynamicSCFaintFingerTargetContrast*32.0)/fingContrast)+0.5))), GAIN_ONE, 235, Dynamic_FFD_gain);
            }
            else
            {
                InitDigitalGainTable((BYTE)((fingContrast == 0)?(GAIN_TWO):((BYTE)(((Glbv.Prv.DynamicFaintFingerTargetContrast*32.0)/fingContrast)+0.5))), GAIN_ONE, 235, Dynamic_FFD_gain);
            }
            for(i=0; i<(int)BufferSize; i++)
            {
                Buffer[i] = Dynamic_FFD_gain[Buffer[i]];
            }
        }
        else
        {
            // static digital gain
            for(i=0; i<(int)BufferSize; i++)
            {
                Buffer[i] = Glbv.Prv.FFD_gain[Buffer[i]];
            }
        }
    }

    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @brief Orient image
 *
 * @param Buffer      Buffer in which is saved the grabbed image.  Must be 
 *                    pre-allocated in memory.  Must be in physical orientation.
 * @param BufferSize  The size of the image buffer
 * @param pLogWinSett The window settings in logical coordinates
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_BADPARAMETER  Window data error
 * @return @ref STERR_MEMORY_MALLOC_FAIL if memory allocation failure
 */
//---------------------------------------------------------------------------
static int AdjustImage_Orientation(unsigned char *Buffer, unsigned long BufferSize, WindowSettings *pLogWinSett)
{
    int iResult = STERR_OK;
    BYTE *TempBuffer = NULL;
    
    // allocate temporary buffer
    TempBuffer = (BYTE*)malloc(sizeof(BYTE)*BufferSize);
    if(TempBuffer == NULL)
    {
        iResult = STERR_MEMORY_MALLOC_FAIL;
        goto Done;
    }
    
    // map physical image to logical image
    memcpy(TempBuffer, Buffer, BufferSize);
    iResult = MapPhysicalToLogicalImage(Buffer, TempBuffer, BufferSize);
    free(TempBuffer);
    if(iResult != STERR_OK)
    {
        goto Done;
    }

    // map physical window settings to logical
    iResult = MapPhysicalToLogicalWindow(pLogWinSett, GlbWin);
    if(iResult != STERR_OK)
    {
        goto Done;
    }

Done:
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STGrab, with unfiltered error returns
 *
 * @param Buffer      Buffer in which to save the grabbed image.  Must be 
 *                    pre-allocated in memory.
 * @param BufferSize  The size of the image buffer
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BAD_STATE if chip in bad state
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_NO_FINGER if no finger detected on sensor
 * @return @ref STERR_BAD_QUALITY_FINGER if a bad quality finger is detected on sensor
 * @return @ref STERR_FAINT_FINGER if a faint finger is detected on sensor
 * @return @ref STERR_LATENT_FINGERPRINT if a latent fingerprint was detected
 * @return @ref STERR_BUFFER_TOO_SMALL if image buffer cannot hold grabbed image
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return @ref STERR_MEMORY_MALLOC_FAIL if memory allocation failure
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
int STiGrab(unsigned char *Buffer, unsigned long BufferSize)
// Internal variation, with unfiltered error returns
{
    int iResult;

    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    // check for NULL parameters
    if(Buffer == NULL)
    {
        return STERR_BADPARAMETER;
    }

    // check buffer size is multiple of 64
    if((BufferSize%64) != 0)
    {
        return STERR_BADPARAMETER;
    }

    // make sure buffer is large enough for current grab settings
    if(BufferSize < (GlbWin->Width*GlbWin->Height))
    {
        return STERR_BUFFER_TOO_SMALL;
    }

    // make sure device is in imaging mode
    if(Glbv.Prv.ChipState != IMAGING_MODE)
    {
        return STERR_BAD_STATE;
    }

    // cap grab size to maximum window size
    if(BufferSize > (XMAX*YMAX))
    {
        BufferSize = (XMAX*YMAX);
    }

    // grab image
    iResult = GrabImage(Buffer, BufferSize);
	
    if(iResult != STERR_OK)
    {
        return iResult;
    }

    // adjust the image
    iResult = AdjustImage(Buffer, BufferSize);

    return iResult;
}

//---------------------------------------------------------------------------
int STGrabBestSett(unsigned char *buf, unsigned long bufsize)
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STGrabBestSett -----------\n");
#endif

    iResult = STiGrabBestSett(buf, bufsize);

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_BAD_STATE) && 
        (iResult!=STERR_BADPARAMETER) && (iResult!=STERR_SENSOR_COMMUNICATION) && 
        (iResult!=STERR_BUFFER_TOO_SMALL) && (iResult!=STERR_NO_FINGER) && 
        (iResult!=STERR_BAD_QUALITY_FINGER) && (iResult!=STERR_FAINT_FINGER) &&
        (iResult!=STERR_LATENT_FINGERPRINT))
    {
        // errors must be visible but not known by the user
        //iResult = STERR_GENERAL_ERROR;
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STGrabBestSett, with unfiltered error returns
 *
 * @param buf         Buffer in which to save the grabbed image.  Must be 
 *                    pre-allocated in memory.
 * @param bufsize     The size of the image buffer
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BAD_STATE if chip in bad state
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_NO_FINGER if no finger detected on sensor
 * @return @ref STERR_BAD_QUALITY_FINGER if a bad quality finger is detected on sensor
 * @return @ref STERR_FAINT_FINGER if a faint finger is detected on sensor
 * @return @ref STERR_BUFFER_TOO_SMALL if image buffer cannot hold grabbed image
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return @ref STERR_SENSOR_NOT_CALIBRATED if sensor needs to be calibrated first
 * @return @ref STERR_MEMORY_MALLOC_FAIL if memory allocation failure
 * @return (other values) Errors returned by internal functions
 */  
//---------------------------------------------------------------------------
int STiGrabBestSett(unsigned char *buf, unsigned long bufsize)
// Internal variation, with unfiltered error returns
{
    int iResult;

    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    // check for NULL parameters
    if(buf == NULL)
    {
        return STERR_BADPARAMETER;
    }

    // check buffer size is multiple of 64
    if((bufsize%64) != 0)
    {
        return STERR_BADPARAMETER;
    }

    // make sure buffer is large enough for current grab settings
    if(bufsize < (GlbWin->Width*GlbWin->Height))
    {
        return STERR_BUFFER_TOO_SMALL;
    }

    // make sure device is in imaging mode
    if(Glbv.Prv.ChipState != IMAGING_MODE)
    {
      return STERR_BAD_STATE;
    }

    // cap grab size to maximum window size
    if(bufsize > (XMAX*YMAX))
    {
        bufsize = (XMAX*YMAX);
    }

    if((Glbv.Prv.rp->ReaderMode == READER_MODE_STD) && (Glbv.Prv.rp->ReaderType != READER_TYPE_STM32))
    {
        // get the best of 32 settings
        iResult = GetBestSettings();
        if(iResult != STERR_OK)
        {
            return iResult;
        }
    }
    else
    {
        // apply default EIM setting
        iResult = GetSelSettings(DEF_EIM_SETTING);
        if(iResult != STERR_OK)
        {
            return iResult;
        }

        // apply EIM offset
        if(Glbv.Prv.rp->EIMOffset != 0)
        {
            iResult = WriteLogicalRegister(OFFSET, Glbv.Prv.rp->EIMOffset, Force);
            if(iResult != STERR_OK)
            {
                return iResult;
            }
        }
        else
        {
            return STERR_SENSOR_NOT_CALIBRATED;
        }
    }

    // grab image
    iResult = GrabImage(buf, bufsize);
	
    if(iResult != STERR_OK)
    {
        return iResult;
    }

    // adjust the image
    iResult = AdjustImage(buf, bufsize);

    return iResult;
}

//---------------------------------------------------------------------------
int STGetBestSett()
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STGetBestSett -----------\n");
#endif

    iResult = STiGetBestSett();

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_BAD_STATE) && 
       (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        // errors must be visible but not known by the user
        //iResult = STERR_GENERAL_ERROR;
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STGetBestSett, with unfiltered error returns
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BAD_STATE if chip in bad state
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return @ref STERR_SENSOR_NOT_CALIBRATED if sensor needs to be calibrated first
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
int STiGetBestSett()
// Internal variation, with unfiltered error returns
{
    int iResult;

    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    // make sure device is in imaging mode
    if(Glbv.Prv.ChipState != IMAGING_MODE)
    {
        return STERR_BAD_STATE;
    }

    if((Glbv.Prv.rp->ReaderMode == READER_MODE_STD) && (Glbv.Prv.rp->ReaderType != READER_TYPE_STM32))
    {
        iResult = GetBestSettings();
    }
    else
    {
        // apply default EIM setting
        iResult = GetSelSettings(DEF_EIM_SETTING);
        if(iResult != STERR_OK)
        {
            return iResult;
        }

        // apply EIM offset
        if(Glbv.Prv.rp->EIMOffset != 0)
        {
            iResult = WriteLogicalRegister(OFFSET, Glbv.Prv.rp->EIMOffset, Force);
        }
        else
        {
            return STERR_SENSOR_NOT_CALIBRATED;
        }
    }

    return iResult;
}

//---------------------------------------------------------------------------
int STGrabSelSett(unsigned char *Buffer, unsigned long BufferSize, int SelSett)
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STGrabSelSett -----------\n");
#endif

    iResult = STiGrabSelSett(Buffer, BufferSize, SelSett);

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_BAD_STATE) && 
       (iResult!=STERR_BADPARAMETER) && (iResult!=STERR_NO_FINGER) && 
       (iResult!=STERR_BAD_QUALITY_FINGER) && (iResult!=STERR_FAINT_FINGER) &&
       (iResult!=STERR_LATENT_FINGERPRINT) && (iResult!=STERR_BUFFER_TOO_SMALL) &&
       (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        // errors must be visible but not known by the user
        //iResult = STERR_GENERAL_ERROR;
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STGrabSelSett, with unfiltered error returns
 *
 * Using this function will change the sensor setting for all future 
 * operations
 *
 * @param Buffer      Buffer in which to save the grabbed image.  Must be 
 *                    pre-allocated in memory.
 * @param BufferSize  The size of the image buffer
 * @param SelSett     The setting at which to set the sensor
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BAD_STATE if chip in bad state
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_NO_FINGER if no finger detected on sensor
 * @return @ref STERR_BAD_QUALITY_FINGER if a bad quality finger is detected on sensor
 * @return @ref STERR_FAINT_FINGER if a faint finger is detected on sensor
 * @return @ref STERR_BUFFER_TOO_SMALL if image buffer cannot hold grabbed image
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return @ref STERR_SENSOR_NOT_CALIBRATED if sensor needs to be calibrated first
 * @return @ref STERR_MEMORY_MALLOC_FAIL if memory allocation failure
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
int STiGrabSelSett(unsigned char *Buffer, unsigned long BufferSize, int SelSett)
// Internal variation, with unfiltered error returns
{
    int iResult;

    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    // check for NULL parameters
    if(Buffer == NULL)
    {
        return STERR_BADPARAMETER;
    }

    // check buffer size is multiple of 64
    if((BufferSize%64) != 0)
    {
        return STERR_BADPARAMETER;
    }

    // make sure buffer is large enough for current grab settings
    if(BufferSize < (GlbWin->Width*GlbWin->Height))
    {
        return STERR_BUFFER_TOO_SMALL;
    }

    // make sure device is in imaging mode
    if(Glbv.Prv.ChipState != IMAGING_MODE)
    {
        return STERR_BAD_STATE;
    }

    // cap grab size to maximum window size
    if(BufferSize > (XMAX*YMAX))
    {
        BufferSize = (XMAX*YMAX);
    }

    if((Glbv.Prv.rp->ReaderMode == READER_MODE_STD) && (Glbv.Prv.rp->ReaderType != READER_TYPE_STM32))
    {
        // validate setting
        if((SelSett<0) || (SelSett>Glbv.Prv.TotalSettings))
        {
            return STERR_BADPARAMETER;
        }

        // get selected setting
        iResult = GetSelSettings(SelSett);
        if(iResult != STERR_OK)
        {
            return iResult;
        }
    }
    else
    {
        // apply default EIM setting
        iResult = GetSelSettings(DEF_EIM_SETTING);
        if(iResult != STERR_OK)
        {
            return iResult;
        }

        // apply EIM offset
        if(Glbv.Prv.rp->EIMOffset != 0)
        {
            iResult = WriteLogicalRegister(OFFSET, Glbv.Prv.rp->EIMOffset, Force);
            if(iResult != STERR_OK)
            {
                return iResult;
            }
        }
        else
        {
            return STERR_SENSOR_NOT_CALIBRATED;
        }
    }

    // grab image
    iResult = GrabImage(Buffer, BufferSize);
	
    if(iResult != STERR_OK)
    {
        return iResult;
    }

    // adjust the image
    iResult = AdjustImage(Buffer, BufferSize);

    return iResult;
}

//---------------------------------------------------------------------------
int STGrabMosaic(unsigned char *Buffer, unsigned long BufferSize)
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STGrabMosaic -----------\n");
#endif

    iResult = STiGrabMosaic(Buffer, BufferSize);

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_BAD_STATE) && 
       (iResult!=STERR_BADPARAMETER) && (iResult!=STERR_BUFFER_TOO_SMALL) && 
       (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        // errors must be visible but not known by the user
        //iResult = STERR_GENERAL_ERROR;
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STGrabMosaic, with unfiltered error returns
 *
 * The small images are captured right in the center of the sensor.
 *
 * @param Buffer      Buffer in which to save the data.  Must be pre-allocated in memory.
 * @param BufferSize  The size of the data buffer
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BAD_STATE if chip in bad state
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_BUFFER_TOO_SMALL if image buffer cannot hold grabbed image
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return @ref STERR_MEMORY_MALLOC_FAIL if memory allocation failure
 * @return @ref STERR_SENSOR_NOT_CALIBRATED if sensor needs to be calibrated first
 * @return (other values) Errors returned by internal functions
 */  
//---------------------------------------------------------------------------
int STiGrabMosaic(unsigned char *Buffer, unsigned long BufferSize)
// Internal variation, with unfiltered error returns
{
    int iResult, currentsetting;
    int EIMBufDimension, EIMBufSize, EIMBufDeadMargin;
    int i, j;
    unsigned char EIMBuffer[4*(MOSAIC_WIDTH*MOSAIC_HEIGHT*1)];  //only 1 setting
    unsigned int X0, Y0;
    WindowSettings winsett;

    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    // check for NULL parameters
    if(Buffer == NULL)
    {
        return STERR_BADPARAMETER;
    }

    // check buffer size is multiple of 64
    if((BufferSize%64) != 0)
    {
        return STERR_BADPARAMETER;
    }

    // make sure device is in imaging mode
    if(Glbv.Prv.ChipState != IMAGING_MODE)
    {
        return STERR_BAD_STATE;
    }

    // cap grab size to maximum window size
    if(BufferSize > (XMAX*YMAX))
    {
        BufferSize = (XMAX*YMAX);
    }

    // grab mosaic in EIM?
    if((Glbv.Prv.rp->ReaderMode == READER_MODE_STD) && (Glbv.Prv.rp->ReaderType != READER_TYPE_STM32))
    {
        // make sure buffer is large enough for current grab settings
        if(BufferSize < (unsigned long)(MOSAIC_WIDTH*MOSAIC_HEIGHT*Glbv.Prv.TotalSettings))
        {
            return STERR_BUFFER_TOO_SMALL;
        }

        // grab mosaic image
        iResult = GrabMultipleImages(Buffer, BufferSize);
        if(iResult != STERR_OK)
        {
            return iResult;
        }

        // adjust the image
        SetTCIFlags(TC_FINGER_TYPE_OVERRIDE | TC_ORIENTATION_OVERRIDE);
        iResult = AdjustImage(Buffer, BufferSize);
        ResetTCIFlags(TC_FINGER_TYPE_OVERRIDE | TC_ORIENTATION_OVERRIDE);

        // apply digital gain compensation
        DigitalGainCompensate(Buffer, BufferSize);
    }
    else
    {
        // make sure buffer is large enough for current grab settings
        if(BufferSize < (unsigned long)(MOSAIC_WIDTH*MOSAIC_HEIGHT*1))
        {
            return STERR_BUFFER_TOO_SMALL;
        }

        // check for ESD event
        iResult = Check_ESD_CommunicationError();
        if(iResult != STERR_OK)
        {
            return iResult;
        }

        SaveSetting(&currentsetting);
        SaveWindowSettings(&winsett);

        // set the centered window size to be 4 times as large as buffer size
        EIMBufDimension = (2*MOSAIC_WIDTH);
        EIMBufSize = (EIMBufDimension*EIMBufDimension);
        X0 = ((GlbWin->MaxWidth-EIMBufDimension)/2);
        Y0 = ((GlbWin->MaxHeight-EIMBufDimension)/2);
        iResult = SetWindow(X0, Y0, EIMBufDimension, EIMBufDimension, 1, 1);
        if(iResult != STERR_OK)
        {
            RestoreSetting(&currentsetting);
            RestoreWindowSettings(&winsett);
            return iResult;
        }

        // apply default EIM setting
        iResult = GetSelSettings(DEF_EIM_SETTING);
        if(iResult != STERR_OK)
        {
            RestoreSetting(&currentsetting);
            RestoreWindowSettings(&winsett);
            return iResult;
        }

        // apply EIM offset
        if(Glbv.Prv.rp->EIMOffset != 0)
        {
            iResult = WriteLogicalRegister(OFFSET, Glbv.Prv.rp->EIMOffset, Force);
            if(iResult != STERR_OK)
            {
                RestoreSetting(&currentsetting);
                RestoreWindowSettings(&winsett);
                return iResult;
            }
        }
        else
        {
            RestoreSetting(&currentsetting);
            RestoreWindowSettings(&winsett);
            return STERR_SENSOR_NOT_CALIBRATED;
        }

        // grab image
        if(GPrv.FirmwareVersion < VERSION_24)
        {
            iResult = OnGrabUSB(EIMBuffer, EIMBufSize, 0);
        }
        else
        {
            iResult = OnGrabUSB(EIMBuffer, EIMBufSize, GRAB_TYPE_NEW_GRAB);
        }
        if(iResult != STERR_OK)
        {
            RestoreSetting(&currentsetting);
            RestoreWindowSettings(&winsett);

            // write EIM gradient offset
            if (Glbv.Prv.rp->EIMOffset != 0)
            {
                WriteLogicalRegister(OFFSET, Glbv.Prv.rp->EIMOffset, Force);
            }
            return iResult;
        }

        // adjust the image
        SetTCIFlags(TC_FINGER_TYPE_OVERRIDE | TC_ORIENTATION_OVERRIDE);
        iResult = AdjustImage(EIMBuffer, EIMBufSize);
        ResetTCIFlags(TC_FINGER_TYPE_OVERRIDE | TC_ORIENTATION_OVERRIDE);
        if(iResult != STERR_OK)
        {
            RestoreSetting(&currentsetting);
            RestoreWindowSettings(&winsett);

            // write EIM gradient offset
            if (Glbv.Prv.rp->EIMOffset != 0)
            {
                WriteLogicalRegister(OFFSET, Glbv.Prv.rp->EIMOffset, Force);
            }
            return iResult;
        }

        // transfer mosaic to usual display (first blacken the usual display)
        memset(Buffer, 1, BufferSize);

        // transfer mosaic from center of grab to usual display
        EIMBufDeadMargin = ((EIMBufDimension-MOSAIC_WIDTH)/2);  //size of path from edge to centered mosaic
        for(i=0; i<EIMBufDimension; i++)
        {
            for(j=0; j<EIMBufDimension; j++)
            {
                // part of the mosaic square?
                if(((i >= EIMBufDeadMargin) && (i < EIMBufDimension-EIMBufDeadMargin)) && 
                   ((j >= EIMBufDeadMargin) && (j < EIMBufDimension-EIMBufDeadMargin)))
                {
                    Buffer[((i-EIMBufDeadMargin)*MOSAIC_WIDTH)+(j-EIMBufDeadMargin)] = EIMBuffer[(i*EIMBufDimension)+j];
                }
            }
        }

        // restore old window settings
        iResult = RestoreSetting(&currentsetting);
        if(iResult != STERR_OK)
        {
            RestoreWindowSettings(&winsett);
            return iResult;
        }
        iResult = RestoreWindowSettings(&winsett);
        if(iResult != STERR_OK)
        {
            return iResult;
        }

        // write EIM gradient offset
        if(Glbv.Prv.rp->EIMOffset != 0)
        {
            iResult = WriteLogicalRegister(OFFSET, Glbv.Prv.rp->EIMOffset, Force);
        }
        else
        {
            return STERR_SENSOR_NOT_CALIBRATED;
        }
    }

    return iResult;
}

//---------------------------------------------------------------------------
int STImage508DPITo500DPI(unsigned char *inBuffer, int inWidth, int inHeight, unsigned char *outBuffer, int *outWidth, int *outHeight)
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STImage508DPITo500DPI -----------\n");
#endif

    iResult = STiImage508DPITo500DPI(inBuffer, inWidth, inHeight, outBuffer, outWidth, outHeight);

    if((iResult!=STERR_OK) && (iResult!=STERR_BADPARAMETER))
    {
        // errors must be visible but not known by the user
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STImage508DPITo500DPI, with unfiltered error returns
 *
 * @param inBuffer    Input 508 DPI image buffer.  Must be pre-allocated in memory.
 * @param inWidth     The width of the input buffer
 * @param inHeight    The height of the input buffer
 * @param outBuffer   Output 500 DPI image buffer.  Must be pre-allocated in memory.
 * @param outWidth    The width of the output buffer (returned)
 * @param outHeight   The height of the output buffer (returned)
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_MEMORY_MALLOC_FAIL if memory allocation failure
 * @return (other values) Errors returned by internal functions
 */  
//---------------------------------------------------------------------------
int STiImage508DPITo500DPI(unsigned char *inBuffer, int inWidth, int inHeight, unsigned char *outBuffer, int *outWidth, int *outHeight)
// Internal variation, with unfiltered error returns
{
    int iResult, x, y, r1, r2, r3, r4, r5;
    int cols, rows, new_cols, new_rows, pixel_skiped;
    float alpha, temp, con_cur, con_nxt, con_nnxt, shift;
    unsigned char *temp_buf_pos;
    unsigned char *tempBuffer = NULL;
    WindowSettings logwin;

    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    // make sure valid parameter pointers
    if(!inBuffer || !outBuffer || !outWidth || !outHeight)
    {
        return STERR_BADPARAMETER;
    }

    // validate input buffer width & height
    iResult = MapPhysicalToLogicalWindow(&logwin, GlbWin);
    if((inWidth<=0) || (inWidth>(int)logwin.MaxWidth))
    {
        return STERR_BADPARAMETER;
    }
    if((inHeight<=0) || (inHeight>(int)logwin.MaxHeight))
    {
        return STERR_BADPARAMETER;
    }

    alpha = (((float)508)/((float)500) - 1);
    cols = inWidth;
    rows = inHeight;
    new_cols = (int)(1.0*cols*500/508);
    new_rows = (int)(1.0*rows*500/508);
    *outWidth = new_cols;
    *outHeight = new_rows;

    r1 = 62;  //(int)((float)1/alpha);
    r2 = 125; //(int)((float)2/alpha);
    r3 = 187; //(int)((float)3/alpha);
    r4 = 250; //(int)((float)4/alpha);
    r5 = 312; //(int)((float)5/alpha);

    tempBuffer = (unsigned char*)malloc(sizeof(unsigned char)*(XMAX*YMAX));
    if(tempBuffer == NULL)
    {
        return STERR_MEMORY_MALLOC_FAIL;
    }

    // do not change original image
    memcpy(tempBuffer, inBuffer, min((XMAX*YMAX),(inWidth*inHeight)));
    temp_buf_pos = tempBuffer;

    for(y=0; y<rows; y++)
    {
        pixel_skiped = 0;
        shift = 0;
        for(x=0; x<new_cols; x++)
        {
            if(x < r1)
            {
                con_cur = (1-(x*alpha));
                con_nxt = (1+alpha-con_cur);
                temp = (con_cur*temp_buf_pos[x]);
                temp += (con_nxt*temp_buf_pos[x+1]);
                temp /= (1+alpha);
                temp_buf_pos[x] = (unsigned char)temp;
            }
            else if(x == r1)
            {
                con_cur = (1-(x*alpha));
                con_nxt = 1;
                con_nnxt = (1+alpha-con_nxt-con_cur);
                temp = (con_cur*temp_buf_pos[x]);
                temp += (con_nxt*temp_buf_pos[x+1]);
                temp += (con_nnxt*temp_buf_pos[x+2]);
                temp /= (1+alpha);
                temp_buf_pos[x] = (unsigned char)temp;
                shift = con_nnxt;
                pixel_skiped++;
            }
            else if(x < r2)
            {
                con_cur = (1-shift-((x-1-r1)*alpha));
                con_nxt = (1+alpha-con_cur);
                temp = (con_cur*temp_buf_pos[pixel_skiped+x]);
                temp += (con_nxt*temp_buf_pos[pixel_skiped+x+1]);
                temp /= (1+alpha);
                temp_buf_pos[x] = (unsigned char)temp;
            }
            else if(x == r2)
            {
                shift = 0;
                pixel_skiped++;
                con_cur = (1-((x-r2)*alpha));
                con_nxt = (1+alpha-con_cur);
                temp = (con_cur*temp_buf_pos[pixel_skiped+x]);
                temp += (con_nxt*temp_buf_pos[pixel_skiped+x+1]);
                temp /= (1+alpha);
                temp_buf_pos[x] = (unsigned char)temp;
            }
            else if(x < r3)
            {
                con_cur = (1-((x-r2)*alpha));
                con_nxt = (1+alpha-con_cur);
                temp = (con_cur*temp_buf_pos[pixel_skiped+x]);
                temp += (con_nxt*temp_buf_pos[pixel_skiped+x+1]);
                temp /= (1+alpha);
                temp_buf_pos[x] = (unsigned char)temp;
            }
            else if(x == r3)
            {
                con_cur = (1-((x-r2)*alpha));
                con_nxt = 1;
                con_nnxt = (1-con_cur-con_nxt);
                temp = (con_cur*temp_buf_pos[pixel_skiped+x]);
                temp += (con_nxt*temp_buf_pos[pixel_skiped+x+1]);
                temp += (con_nnxt*temp_buf_pos[pixel_skiped+x+2]);
                temp /= (1+alpha);
                temp_buf_pos[x] = (unsigned char)temp;
                shift = con_nnxt;
                pixel_skiped++;
            }
            else if(x < r4)
            {
                con_cur = (1-shift-((x-1-r3)*alpha));
                con_nxt = (1+alpha-con_cur);
                temp = (con_cur*temp_buf_pos[pixel_skiped+x]);
                temp += (con_nxt*temp_buf_pos[pixel_skiped+x+1]);
                temp /= (1+alpha);
                temp_buf_pos[x] = (unsigned char)temp;
            }
            else if(x == r4)
            {
                shift = 0;
                pixel_skiped++;
                con_cur = (1-((x-r4)*alpha));
                con_nxt = (1+alpha-con_cur);
                temp = (con_cur*temp_buf_pos[pixel_skiped+x]);
                temp += (con_nxt*temp_buf_pos[pixel_skiped+x+1]);
                temp /= (1+alpha);
                temp_buf_pos[x] = (unsigned char)temp;
            }
            else
            {
                con_cur = (1-((x-r4)*alpha));
                con_nxt = (1+alpha-con_cur);
                temp = (con_cur*temp_buf_pos[pixel_skiped+x]);
                temp += (con_nxt*temp_buf_pos[pixel_skiped+x+1]);
                temp /= (1+alpha);
                temp_buf_pos[x] = (unsigned char)temp;
            }
        }
        temp_buf_pos += cols;
    }

    pixel_skiped = 0;
    shift = 0;
    temp_buf_pos = tempBuffer;
    for(y=0; y<new_rows; y++)
    {
        if(y < r1)
        {
            con_cur = (1-(y*alpha));
            con_nxt = (1+alpha-con_cur);
            for(x=0; x<new_cols; x++)
            {
                temp = (con_cur*temp_buf_pos[x]);
                temp += (con_nxt*temp_buf_pos[cols+x]);
                temp /= (1+alpha);
                outBuffer[x+(new_cols*y)] = (unsigned char)temp;
            }
        }
        else if(y == r1)
        {
            con_cur = (1-(y*alpha));
            con_nxt = 1;
            con_nnxt = (1+alpha-con_nxt-con_cur);
            for(x=0; x<new_cols; x++)
            {
                temp = (con_cur*temp_buf_pos[x]);
                temp += (con_nxt*temp_buf_pos[cols+x]);
                temp += (con_nnxt*temp_buf_pos[(2*cols)+x]);
                temp /= (1+alpha);
                outBuffer[x+(new_cols*y)] = (unsigned char)temp;
            }
            shift = con_nnxt;
            pixel_skiped++;
        }
        else if(y < r2)
        {
            con_cur = (1-shift-((y-1-r1)*alpha));
            con_nxt = (1+alpha-con_cur);
            for(x=0; x<new_cols; x++)
            {
                temp = (con_cur*temp_buf_pos[(pixel_skiped*cols)+x]);
                temp += (con_nxt*temp_buf_pos[((pixel_skiped+1)*cols)+x]);
                temp /= (1+alpha);
                outBuffer[x+(new_cols*y)] = (unsigned char)temp;
            }
        }
        else if(y == r2)
        {
            shift = 0;
            pixel_skiped++;
            con_cur = (1-((y-r2)*alpha));
            con_nxt = (1+alpha-con_cur);
            for(x=0; x<new_cols; x++)
            {
                temp = (con_cur*temp_buf_pos[(pixel_skiped*cols)+x]);
                temp += (con_nxt*temp_buf_pos[((pixel_skiped+1)*cols)+x]);
                temp /= (1+alpha);
                outBuffer[x+(new_cols*y)] = (unsigned char)temp;
            }
        }
        else if(y < r3)
        {
            con_cur = (1-((y-r2)*alpha));
            con_nxt = (1+alpha-con_cur);
            for(x=0; x<new_cols; x++)
            {
                temp = (con_cur*temp_buf_pos[(pixel_skiped*cols)+x]);
                temp += (con_nxt*temp_buf_pos[((pixel_skiped+1)*cols)+x]);
                temp /= (1+alpha);
                outBuffer[x+(new_cols*y)] = (unsigned char)temp;
            }
        }
        else if(y == r3)
        {
            con_cur = (1-((y-r2)*alpha));
            con_nxt = 1;
            con_nnxt = (1-con_cur-con_nxt);
            for(x=0; x<new_cols; x++)
            {
                temp = (con_cur*temp_buf_pos[(pixel_skiped*cols)+x]);
                temp += (con_nxt*temp_buf_pos[((pixel_skiped+1)*cols)+x]);
                temp += (con_nnxt*temp_buf_pos[((pixel_skiped+2)*cols)+x]);
                temp /= (1+alpha);
                outBuffer[x+(new_cols*y)] = (unsigned char)temp;
            }
            shift = con_nnxt;
            pixel_skiped++;
        }
        else if(y < r4)
        {
            con_cur = (1-shift-((y-1-r3)*alpha));
            con_nxt = (1+alpha-con_cur);
            for(x=0; x<new_cols; x++)
            {
                temp = (con_cur*temp_buf_pos[(pixel_skiped*cols)+x]);
                temp += (con_nxt*temp_buf_pos[((pixel_skiped+1)*cols)+x]);
                temp /= (1+alpha);
                outBuffer[x+(new_cols*y)] = (unsigned char)temp;
            }
        }
        else if(y == r4)
        {
            shift = 0;
            pixel_skiped++;
            con_cur = (1-((y-r4)*alpha));
            con_nxt = (1+alpha-con_cur);
            for(x=0; x<new_cols; x++)
            {
                temp = (con_cur*temp_buf_pos[(pixel_skiped*cols)+x]);
                temp += (con_nxt*temp_buf_pos[((pixel_skiped+1)*cols)+x]);
                temp /= (1+alpha);
                outBuffer[x+(new_cols*y)] = (unsigned char)temp;
            }
        }
        else if(y < r5)
        {
            con_cur = (1-((y-r4)*alpha));
            con_nxt = (1+alpha-con_cur);
            for(x=0; x<new_cols; x++)
            {
                temp = (con_cur*temp_buf_pos[(pixel_skiped*cols)+x]);
                temp += (con_nxt*temp_buf_pos[((pixel_skiped+1)*cols)+x]);
                temp /= (1+alpha);
                outBuffer[x+(new_cols*y)] = (unsigned char)temp;
            }
        }
        else if(y == r5)
        {
            con_cur = (1-((y-r4)*alpha));
            con_nxt = 1;
            con_nnxt = (1-con_cur-con_nxt);
            for(x=0; x<new_cols; x++)
            {
                temp = (con_cur*temp_buf_pos[(pixel_skiped*cols)+x]);
                temp += (con_nxt*temp_buf_pos[((pixel_skiped+1)*cols)+x]);
                temp += (con_nnxt*temp_buf_pos[((pixel_skiped+2)*cols)+x]);
                temp /= (1+alpha);
                outBuffer[x+(new_cols*y)] = (unsigned char)temp;
            }
            shift = con_nnxt;
            pixel_skiped++;
        }
        else
        {
            con_cur = (1-shift-((y-1-r5)*alpha));
            con_nxt = (1+alpha-con_cur);
            for(x=0; x<new_cols; x++)
            {
                temp = (con_cur*temp_buf_pos[(pixel_skiped*cols)+x]);
                temp += (con_nxt*temp_buf_pos[((pixel_skiped+1)*cols)+x]);
                temp /= (1+alpha);
                outBuffer[x+(new_cols*y)] = (unsigned char)temp;
            }
        }
        temp_buf_pos += cols;
    }

    free(tempBuffer);
    return iResult;
}

//---------------------------------------------------------------------------
int STImageToANSI381(unsigned char *inBuffer, int inWidth, int inHeight, unsigned char *outBuffer, unsigned long outBufferSize)
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STImageToANSI381 -----------\n");
#endif

    iResult = STiImageToANSI381(inBuffer, inWidth, inHeight, outBuffer, outBufferSize);

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_BADPARAMETER))
    {
        // errors must be visible but not known by the user
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STImageToANSI381, with unfiltered error returns
 *
 * @param inBuffer      Input image buffer.  Must be pre-allocated in memory.
 * @param inWidth       The width of the input buffer
 * @param inHeight      The height of the input buffer
 * @param outBuffer     Output ANSI 381 image buffer.  Must be pre-allocated in memory.
 * @param outBufferSize The size (in bytes) of the output ANSI 381 image buffer
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
int STiImageToANSI381(unsigned char *inBuffer, int inWidth, int inHeight, unsigned char *outBuffer, unsigned long outBufferSize)
// Internal variation, with unfiltered error returns
{
    unsigned long tmp;
    unsigned char vendorCode;

    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    // make sure valid parameter pointers
    if(!inBuffer || !outBuffer || (outBufferSize<(unsigned long)(50+(inWidth*inHeight))))
    {
        return STERR_BADPARAMETER;
    }

    // determine the vendor code
    if((inWidth==256) && (inHeight==360))
    {
        // TCS1C
        vendorCode = 38;
    }
    else if((inWidth==208) && (inHeight==288))
    {
        // TCS2C
        vendorCode = 27;
    }
    else
    {
        // unknown
        return STERR_BADPARAMETER;
    }

    //////////////////////////////////////////////////////////////////
    // General Header (36 bytes)

    // Format Identifier (4B) - "FIR" (Finger Image Record)
    outBuffer[0]  = 0x46;
    outBuffer[1]  = 0x49;
    outBuffer[2]  = 0x52;
    outBuffer[3]  = 0x00;
    // Version Number (4B) - "010"
    outBuffer[4]  = 0x30;
    outBuffer[5]  = 0x31;
    outBuffer[6]  = 0x30;
    outBuffer[7]  = 0x00;
    // Record length (6B) - 36+(Number Views)*[14+(image width)*(image height)]
    tmp = (36+(1*(14+(inWidth*inHeight))));
    outBuffer[8]  = 0x0;
    outBuffer[9]  = 0x0;
    outBuffer[10] = BF_GETBYTE3(tmp);
    outBuffer[11] = BF_GETBYTE2(tmp);
    outBuffer[12] = BF_GETBYTE1(tmp);
    outBuffer[13] = BF_GETBYTE0(tmp);
    // CBEFF Product Identifier (4B) - owner of the encoding equipment
    outBuffer[14] = 0x0;
    outBuffer[15] = 0x12;
    outBuffer[16] = 0x0;
    outBuffer[17] = 0x0;
    // Capture device ID (2B) - id of device that captured fingerprint image
    outBuffer[18] = 0x0;    // only 12 bits
    outBuffer[19] = vendorCode;
    // Image acquisition level (2B)
    outBuffer[20] = 0x0;
    outBuffer[21] = 0x1F;
    // Number of fingers (1B)
    outBuffer[22] = 0x1;
    // Scale units (1B) - 0x1 is pixels per inch, 0x2 is pixels per cm
    outBuffer[23] = 0x1; //ppi
    // Horizontal scan resolution (2B)
    outBuffer[24] = 0x01;
    outBuffer[25] = 0xFC;
    // Vertical scan resolution (2B)
    outBuffer[26] = 0x01;
    outBuffer[27] = 0xFC;
    // Horizontal image resolution (2B)
    outBuffer[28] = 0x01;
    outBuffer[29] = 0xFC;
    // Vertical image resolution (2B)
    outBuffer[30] = 0x01;
    outBuffer[31] = 0xFC;
    // Pixel Depth (1B) - bits per pixel
    outBuffer[32] = 0x8;
    // Image compression algorithm (1B) - which image compression algorighm (0x0 is unknown)
    outBuffer[33] = 0x0;
    // Reserved (2B)
    outBuffer[34] = 0x0;
    outBuffer[35] = 0x0;

    //////////////////////////////////////////////////////////////////
    // Finger Image header (14 bytes)

    // Length of finger data block (4B) - (14 + (image width)*(image height));
    tmp = (14+(inWidth*inHeight));
    outBuffer[36] = BF_GETBYTE3(tmp);
    outBuffer[37] = BF_GETBYTE2(tmp);
    outBuffer[38] = BF_GETBYTE1(tmp);
    outBuffer[39] = BF_GETBYTE0(tmp);
    // Finger position (1B) - which finger is represented (0x0 is unknown)
    outBuffer[40] = 0x0;
    // Count of views (1B)
    outBuffer[41] = 0x1;
    // View number (1B)
    outBuffer[42] = 0x1;
    // Finger image quality (1B)
    outBuffer[43] = 0xFE;
    // Impression type (1B) - 0x0 is live-scan plain
    outBuffer[44] = 0x0;
    // Horizontal line length (2B) - width of image
    outBuffer[45] = BF_GETBYTE1(inWidth);
    outBuffer[46] = BF_GETBYTE0(inWidth);
    // Vertical line length (2B) - height of image
    outBuffer[47] = BF_GETBYTE1(inHeight);
    outBuffer[48] = BF_GETBYTE0(inHeight);
    // Reserved (1B)
    outBuffer[49] = 0x0;

    //////////////////////////////////////////////////////////////////
    // Finger data ((image width)*(image height) bytes)

    for(tmp=0; tmp<(unsigned long)(inWidth*inHeight); tmp++)
    {
        outBuffer[50+tmp] = inBuffer[tmp];
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
int STInitSensorPar(int SelSett)
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STInitSensorPar -----------\n");
#endif

    iResult = STiInitSensorPar(SelSett);

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_BADPARAMETER) && 
       (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        // errors must be visible but not known by the user
        //iResult = STERR_GENERAL_ERROR;
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STInitSensorPar, with unfiltered error returns
 *
 * Using this function will change the sensor setting for all future 
 * operations
 *
 * @param SelSett     The setting at which to set the sensor
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return @ref STERR_SENSOR_NOT_CALIBRATED if sensor needs to be calibrated first
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
int STiInitSensorPar(int SelSett)
// Internal variation, with unfiltered error returns
{
    int iResult;

    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    // validate setting
    if(SelSett < 0 || SelSett >= Glbv.Prv.TotalSettings)
    {
        return STERR_BADPARAMETER;
    }

    // only 1 setting is supported in EIM
    if((Glbv.Prv.rp->ReaderMode != READER_MODE_STD) || (Glbv.Prv.rp->ReaderType == READER_TYPE_STM32))
    {
        // apply default EIM setting
        iResult = GetSelSettings(DEF_EIM_SETTING);
        if(iResult != STERR_OK)
        {
            return iResult;
        }

        // apply EIM offset
        if(Glbv.Prv.rp->EIMOffset != 0)
        {
            iResult = WriteLogicalRegister(OFFSET, Glbv.Prv.rp->EIMOffset, Force);
        }
        else
        {
            return STERR_SENSOR_NOT_CALIBRATED;
        }
    }
    else
    {
        // apply selected setting
        iResult = GetSelSettings(SelSett);
    }

    return iResult;
}

//---------------------------------------------------------------------------
int STGetCurrentSetting()
{
    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    return Glbv.Prv.CurrentSetting;
}

//---------------------------------------------------------------------------
int STGetChipState()
{
    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    return (int)Glbv.Prv.ChipState;
}

//---------------------------------------------------------------------------
int STSetChipState(int data)
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STSetChipState -----------\n");
#endif

    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    iResult = STiSetChipState(data);

    if((iResult!=STERR_OK) && (iResult!=STERR_BADPARAMETER) && (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        // errors must be visible but not known by the user
        //iResult = STERR_GENERAL_ERROR;
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STSetChipState, with unfiltered error returns
 *
 * @param data  The state to set the chip to - @ref chipstates
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
int STiSetChipState(int data)
// Internal variation, with unfiltered error returns
{
    int iResult;

    switch(data)
    {
    case SLEEP_MODE:
        iResult = WriteLogicalRegister(SUPPLYOFF, 0, NoForce);
        if(iResult != STERR_OK)
        {
            return iResult;
        }
        iResult = WriteLogicalRegister(LSD, 0, NoForce);
        if(iResult != STERR_OK)
        {
            return iResult;
        }
        iResult = WriteLogicalRegister(SHDWOSC, 0, NoForce);
        if(iResult != STERR_OK)
        {
            return iResult;
        }
        iResult = WriteLogicalRegister(DETPD, 0, Force);
        if(iResult != STERR_OK)
        {
            return iResult;
        }
        Glbv.Prv.ChipState = (BYTE)SLEEP_MODE;
        break;
    case IMAGING_MODE:
        iResult = WriteLogicalRegister(SUPPLYOFF, 1, NoForce);
        if(iResult != STERR_OK)
        {
            return iResult;
        }
        iResult = WriteLogicalRegister(LSD, 1, NoForce);
        if(iResult != STERR_OK)
        {
            return iResult;
        }
        iResult = WriteLogicalRegister(SHDWOSC, 1, NoForce);
        if(iResult != STERR_OK)
        {
            return iResult;
        }
        iResult = WriteLogicalRegister(DETPD, 1, Force);
        if(iResult != STERR_OK)
        {
            return iResult;
        }
        Glbv.Prv.ChipState = (BYTE)IMAGING_MODE;
        break;
    default:
        return STERR_BADPARAMETER;
    }

    // check for ESD event
    iResult = Check_ESD_CommunicationError();

    return iResult;
}

//---------------------------------------------------------------------------
int STGetTotalSettings()
{
    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    // only 1 setting is supported in EIM
    if((Glbv.Prv.rp->ReaderMode != READER_MODE_STD) || (Glbv.Prv.rp->ReaderType == READER_TYPE_STM32))
    {
        return 1;
    }

    return Glbv.Prv.TotalSettings;
}

//---------------------------------------------------------------------------
int STGetSessionCfg(unsigned int SessionVersion, void *pSessionCfg, unsigned int SessionCfgSize)
{
    int iResult = STERR_OK;

    // check if STInitialize has succesfully completed
    if((Glbv.Prv.PP_IntStatus == NotInUse) || (Glbv.Prv.PP_IntStatus == PP_Terminate))
    {
        return STERR_NODEVICE;
    }

    // check for NULL parameters
    if(pSessionCfg == NULL)
    {
        return STERR_BADPARAMETER;
    }

    // get the session configuration based on the desired version
    switch(SessionVersion)
    {
        case SESSION_CFG_V1:
            if(SessionCfgSize != sizeof(SessionCfgStruct_V1))
            {
                // session configuration structure is not of expected size
                iResult = STERR_BADPARAMETER;
            }
            else
            {
                // load the current session configuration into the structure
                ((SessionCfgStruct_V1*)pSessionCfg)->FaintFinger_MinContrast = Glbv.Prv.StaticFaintFingerThreshold;
                ((SessionCfgStruct_V1*)pSessionCfg)->FaintFinger_BkgDiffTolerance = Glbv.Prv.FingerTriggerThreshold;
                ((SessionCfgStruct_V1*)pSessionCfg)->FaintFinger_ApplyDigitalGain = Glbv.Prv.ApplyDigitalGain;
            }
            break;
        case SESSION_CFG_V2:
            if(SessionCfgSize != sizeof(SessionCfgStruct_V2))
            {
                // session configuration structure is not of expected size
                iResult = STERR_BADPARAMETER;
            }
            else
            {
                // load the current session configuration into the structure
                ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_StaticFaintContrastTH = Glbv.Prv.StaticFaintFingerThreshold;
                ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_DynamicNoFingerContrastTH = Glbv.Prv.DynamicNoFingerThreshold;
                ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_DynamicFaintContrastTH = Glbv.Prv.DynamicFaintFingerThreshold;
                ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_DynamicTargetContrast = Glbv.Prv.DynamicFaintFingerTargetContrast;
                ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_FingerTriggerTH = Glbv.Prv.FingerTriggerThreshold;
                ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_BoxMinTH = Glbv.Prv.FingerBoxMinThreshold;
                ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_UseDynamicDigitalGain = Glbv.Prv.UseDynamicDigitalGainForFaintFinger;
                ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_ApplyDigitalGain = Glbv.Prv.ApplyDigitalGain;
                ((SessionCfgStruct_V2*)pSessionCfg)->OnTheFlyRepairEnabled = Glbv.Prv.OnTheFlyRepairEnabled;
                ((SessionCfgStruct_V2*)pSessionCfg)->STM32_EncryptionEnabled = Glbv.Prv.STM32_EncryptionEnabled;
            }
            break;
        case SESSION_CFG_V3:
            if(SessionCfgSize != sizeof(SessionCfgStruct_V3))
            {
                // session configuration structure is not of expected size
                iResult = STERR_BADPARAMETER;
            }
            else
            {
                // load the current session configuration into the structure
                ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_StaticFaintContrastTH = Glbv.Prv.StaticFaintFingerThreshold;
                ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_DynamicNoFingerContrastTH = Glbv.Prv.DynamicNoFingerThreshold;
                ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_DynamicFaintContrastTH = Glbv.Prv.DynamicFaintFingerThreshold;
                ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_DynamicTargetContrast = Glbv.Prv.DynamicFaintFingerTargetContrast;
                ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_FingerTriggerTH = Glbv.Prv.FingerTriggerThreshold;
                ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_BoxMinTH = Glbv.Prv.FingerBoxMinThreshold;
                ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_StaticSCFaintContrastTH = Glbv.Prv.StaticSCFaintFingerThreshold;
                ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_DynamicSCNoFingerContrastTH = Glbv.Prv.DynamicSCNoFingerThreshold;
                ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_DynamicSCFaintContrastTH = Glbv.Prv.DynamicSCFaintFingerThreshold;
                ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_DynamicSCTargetContrast = Glbv.Prv.DynamicSCFaintFingerTargetContrast;
                ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_SCFingerTriggerTH = Glbv.Prv.SCFingerTriggerThreshold;
                ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_SCBoxMinTH = Glbv.Prv.SCFingerBoxMinThreshold;
                ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_UseDynamicDigitalGain = Glbv.Prv.UseDynamicDigitalGainForFaintFinger;
                ((SessionCfgStruct_V3*)pSessionCfg)->ApplyDigitalGain = Glbv.Prv.ApplyDigitalGain;
                ((SessionCfgStruct_V3*)pSessionCfg)->OnTheFlyRepairEnabled = Glbv.Prv.OnTheFlyRepairEnabled;
                ((SessionCfgStruct_V3*)pSessionCfg)->STM32_EncryptionEnabled = Glbv.Prv.STM32_EncryptionEnabled;
            }
            break;
        case SESSION_CFG_V4:
            if(SessionCfgSize != sizeof(SessionCfgStruct_V4))
            {
                // session configuration structure is not of expected size
                iResult = STERR_BADPARAMETER;
            }
            else
            {
                // load the current session configuration into the structure
                ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_StaticFaintContrastTH = Glbv.Prv.StaticFaintFingerThreshold;
                ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_DynamicNoFingerContrastTH = Glbv.Prv.DynamicNoFingerThreshold;
                ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_DynamicFaintContrastTH = Glbv.Prv.DynamicFaintFingerThreshold;
                ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_DynamicTargetContrast = Glbv.Prv.DynamicFaintFingerTargetContrast;
                ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_FingerTriggerTH = Glbv.Prv.FingerTriggerThreshold;
                ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_BoxMinTH = Glbv.Prv.FingerBoxMinThreshold;
                ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_StaticSCFaintContrastTH = Glbv.Prv.StaticSCFaintFingerThreshold;
                ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_DynamicSCNoFingerContrastTH = Glbv.Prv.DynamicSCNoFingerThreshold;
                ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_DynamicSCFaintContrastTH = Glbv.Prv.DynamicSCFaintFingerThreshold;
                ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_DynamicSCTargetContrast = Glbv.Prv.DynamicSCFaintFingerTargetContrast;
                ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_SCFingerTriggerTH = Glbv.Prv.SCFingerTriggerThreshold;
                ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_SCBoxMinTH = Glbv.Prv.SCFingerBoxMinThreshold;
                ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_UseDynamicDigitalGain = Glbv.Prv.UseDynamicDigitalGainForFaintFinger;
                ((SessionCfgStruct_V4*)pSessionCfg)->ApplyDigitalGain = Glbv.Prv.ApplyDigitalGain;
                ((SessionCfgStruct_V4*)pSessionCfg)->OnTheFlyRepairEnabled = Glbv.Prv.OnTheFlyRepairEnabled;
                ((SessionCfgStruct_V4*)pSessionCfg)->STM32_EncryptionEnabled = Glbv.Prv.STM32_EncryptionEnabled;
            }
            break;
        default:
            // unknown version
            iResult = STERR_BADPARAMETER;
    }

    return iResult;
}

//---------------------------------------------------------------------------
int STSetSessionCfg(unsigned int SessionVersion, void *pSessionCfg, unsigned int SessionCfgSize)
{
    int iResult = STERR_OK;
    int fwRev;

    // check if STInitialize has succesfully completed
    if((Glbv.Prv.PP_IntStatus==NotInUse) || (Glbv.Prv.PP_IntStatus==PP_Terminate))
    {
        return STERR_NODEVICE;
    }

    // check for NULL parameters
    if(pSessionCfg == NULL)
    {
        return STERR_BADPARAMETER;
    }

    // set the session configuration based on the desired version
    switch(SessionVersion)
    {
        case SESSION_CFG_V1:
            if(SessionCfgSize != sizeof(SessionCfgStruct_V1))
            {
                // session configuration structure is not of expected size
                iResult = STERR_BADPARAMETER;
            }
            else
            {
                // set the current session configuration to the desired
                Glbv.Prv.StaticFaintFingerThreshold = ((SessionCfgStruct_V1*)pSessionCfg)->FaintFinger_MinContrast;
                Glbv.Prv.FingerTriggerThreshold = ((SessionCfgStruct_V1*)pSessionCfg)->FaintFinger_BkgDiffTolerance;

                Glbv.Prv.ApplyDigitalGain = ((SessionCfgStruct_V1*)pSessionCfg)->FaintFinger_ApplyDigitalGain;
                if(Glbv.Prv.ApplyDigitalGain)
                {
                    ResetTCIFlags(TC_DIG_GAIN_OVERRIDE);
                }
                else
                {
                    SetTCIFlags(TC_DIG_GAIN_OVERRIDE);
                }
            }
            break;
        case SESSION_CFG_V2:
            if(SessionCfgSize != sizeof(SessionCfgStruct_V2))
            {
                // session configuration structure is not of expected size
                iResult = STERR_BADPARAMETER;
            }
            else
            {
                // set the current session configuration to the desired
                Glbv.Prv.StaticFaintFingerThreshold = ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_StaticFaintContrastTH;
                Glbv.Prv.DynamicNoFingerThreshold = ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_DynamicNoFingerContrastTH;
                Glbv.Prv.DynamicFaintFingerThreshold = ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_DynamicFaintContrastTH;
                Glbv.Prv.DynamicFaintFingerTargetContrast = ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_DynamicTargetContrast;
                Glbv.Prv.FingerTriggerThreshold = ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_FingerTriggerTH;
                Glbv.Prv.FingerBoxMinThreshold = ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_BoxMinTH;
                Glbv.Prv.UseDynamicDigitalGainForFaintFinger = ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_UseDynamicDigitalGain;

                Glbv.Prv.ApplyDigitalGain = ((SessionCfgStruct_V2*)pSessionCfg)->FaintFinger_ApplyDigitalGain;
                if(Glbv.Prv.ApplyDigitalGain)
                {
                    ResetTCIFlags(TC_DIG_GAIN_OVERRIDE);
                }
                else
                {
                    SetTCIFlags(TC_DIG_GAIN_OVERRIDE);
                }

                Glbv.Prv.OnTheFlyRepairEnabled = ((SessionCfgStruct_V2*)pSessionCfg)->OnTheFlyRepairEnabled;
                if(Glbv.Prv.OnTheFlyRepairEnabled)
                {
                    ResetTCIFlags(TC_OTF_OVERRIDE);
                }
                else
                {
                    SetTCIFlags(TC_OTF_OVERRIDE);
                }

                if(Glbv.Prv.rp->ReaderType == READER_TYPE_STM32)
                {
                    if((Glbv.Prv.STM32_EncryptionEnabled==TRUE) && (((SessionCfgStruct_V2*)pSessionCfg)->STM32_EncryptionEnabled==FALSE))
                    {
                        // cannot turn STM32 encryption off if currently on
                        iResult = STERR_NOT_SUPPORTED;
                    }
                    else if((Glbv.Prv.STM32_EncryptionEnabled==FALSE) && (((SessionCfgStruct_V2*)pSessionCfg)->STM32_EncryptionEnabled==TRUE))
                    {
                        // check if firmware supports STM32 encryption
                        iResult = ReadLogicalRegister(FW_REV, &fwRev, Force);
                        if(iResult == STERR_OK)
                        {
                            if(fwRev >= 0x28)
                            {
                                iResult = OnSendCmdTableUSB(ST_TOUCH_SET_ENCRYPTION, 0, NULL, 0);
                                if(iResult == STERR_OK)
                                {
                                    Glbv.Prv.STM32_EncryptionEnabled = ((SessionCfgStruct_V2*)pSessionCfg)->STM32_EncryptionEnabled;
                                }
                            }
                            else
                            {
                                iResult = STERR_NOT_SUPPORTED;
                            }
                        }
                    }
                }
                else
                {
                    // ignore for non-STM32
                    Glbv.Prv.STM32_EncryptionEnabled = ((SessionCfgStruct_V2*)pSessionCfg)->STM32_EncryptionEnabled;
                }
            }
            break;
        case SESSION_CFG_V3:
            if(SessionCfgSize != sizeof(SessionCfgStruct_V3))
            {
                // session configuration structure is not of expected size
                iResult = STERR_BADPARAMETER;
            }
            else
            {
                // set the current session configuration to the desired
                Glbv.Prv.StaticFaintFingerThreshold = ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_StaticFaintContrastTH;
                Glbv.Prv.DynamicNoFingerThreshold = ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_DynamicNoFingerContrastTH;
                Glbv.Prv.DynamicFaintFingerThreshold = ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_DynamicFaintContrastTH;
                Glbv.Prv.DynamicFaintFingerTargetContrast = ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_DynamicTargetContrast;
                Glbv.Prv.FingerTriggerThreshold = ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_FingerTriggerTH;
                Glbv.Prv.FingerBoxMinThreshold = ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_BoxMinTH;
                Glbv.Prv.StaticSCFaintFingerThreshold = ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_StaticSCFaintContrastTH;
                Glbv.Prv.DynamicSCNoFingerThreshold = ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_DynamicSCNoFingerContrastTH;
                Glbv.Prv.DynamicSCFaintFingerThreshold = ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_DynamicSCFaintContrastTH;
                Glbv.Prv.DynamicSCFaintFingerTargetContrast = ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_DynamicSCTargetContrast;
                Glbv.Prv.SCFingerTriggerThreshold = ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_SCFingerTriggerTH;
                Glbv.Prv.SCFingerBoxMinThreshold = ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_SCBoxMinTH;
                Glbv.Prv.UseDynamicDigitalGainForFaintFinger = ((SessionCfgStruct_V3*)pSessionCfg)->FaintFinger_UseDynamicDigitalGain;

                Glbv.Prv.ApplyDigitalGain = ((SessionCfgStruct_V3*)pSessionCfg)->ApplyDigitalGain;
                if(Glbv.Prv.ApplyDigitalGain)
                {
                    ResetTCIFlags(TC_DIG_GAIN_OVERRIDE);
                }
                else
                {
                    SetTCIFlags(TC_DIG_GAIN_OVERRIDE);
                }

                Glbv.Prv.OnTheFlyRepairEnabled = ((SessionCfgStruct_V3*)pSessionCfg)->OnTheFlyRepairEnabled;
                if(Glbv.Prv.OnTheFlyRepairEnabled)
                {
                    ResetTCIFlags(TC_OTF_OVERRIDE);
                }
                else
                {
                    SetTCIFlags(TC_OTF_OVERRIDE);
                }

                if(Glbv.Prv.rp->ReaderType == READER_TYPE_STM32)
                {
                    if((Glbv.Prv.STM32_EncryptionEnabled==TRUE) && (((SessionCfgStruct_V3*)pSessionCfg)->STM32_EncryptionEnabled==FALSE))
                    {
                        // cannot turn STM32 encryption off if currently on
                        iResult = STERR_NOT_SUPPORTED;
                    }
                    else if((Glbv.Prv.STM32_EncryptionEnabled==FALSE) && (((SessionCfgStruct_V3*)pSessionCfg)->STM32_EncryptionEnabled==TRUE))
                    {
                        // check if firmware supports STM32 encryption
                        iResult = ReadLogicalRegister(FW_REV, &fwRev, Force);
                        if(iResult == STERR_OK)
                        {
                            if(fwRev >= 0x28)
                            {
                                iResult = OnSendCmdTableUSB(ST_TOUCH_SET_ENCRYPTION, 0, NULL, 0);
                                if(iResult == STERR_OK)
                                {
                                    Glbv.Prv.STM32_EncryptionEnabled = ((SessionCfgStruct_V3*)pSessionCfg)->STM32_EncryptionEnabled;
                                }
                            }
                            else
                            {
                                iResult = STERR_NOT_SUPPORTED;
                            }
                        }
                    }
                }
                else
                {
                    // ignore for non-STM32
                    Glbv.Prv.STM32_EncryptionEnabled = ((SessionCfgStruct_V3*)pSessionCfg)->STM32_EncryptionEnabled;
                }
            }
            break;
        case SESSION_CFG_V4:
            if(SessionCfgSize != sizeof(SessionCfgStruct_V4))
            {
                // session configuration structure is not of expected size
                iResult = STERR_BADPARAMETER;
            }
            else
            {
                // set the current session configuration to the desired
                Glbv.Prv.StaticFaintFingerThreshold = ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_StaticFaintContrastTH;
                Glbv.Prv.DynamicNoFingerThreshold = ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_DynamicNoFingerContrastTH;
                Glbv.Prv.DynamicFaintFingerThreshold = ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_DynamicFaintContrastTH;
                Glbv.Prv.DynamicFaintFingerTargetContrast = ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_DynamicTargetContrast;
                Glbv.Prv.FingerTriggerThreshold = ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_FingerTriggerTH;
                Glbv.Prv.FingerBoxMinThreshold = ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_BoxMinTH;
                Glbv.Prv.StaticSCFaintFingerThreshold = ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_StaticSCFaintContrastTH;
                Glbv.Prv.DynamicSCNoFingerThreshold = ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_DynamicSCNoFingerContrastTH;
                Glbv.Prv.DynamicSCFaintFingerThreshold = ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_DynamicSCFaintContrastTH;
                Glbv.Prv.DynamicSCFaintFingerTargetContrast = ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_DynamicSCTargetContrast;
                Glbv.Prv.SCFingerTriggerThreshold = ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_SCFingerTriggerTH;
                Glbv.Prv.SCFingerBoxMinThreshold = ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_SCBoxMinTH;
                Glbv.Prv.UseDynamicDigitalGainForFaintFinger = ((SessionCfgStruct_V4*)pSessionCfg)->FaintFinger_UseDynamicDigitalGain;

                Glbv.Prv.ApplyDigitalGain = ((SessionCfgStruct_V4*)pSessionCfg)->ApplyDigitalGain;
                if(Glbv.Prv.ApplyDigitalGain)
                {
                    ResetTCIFlags(TC_DIG_GAIN_OVERRIDE);
                }
                else
                {
                    SetTCIFlags(TC_DIG_GAIN_OVERRIDE);
                }

                Glbv.Prv.OnTheFlyRepairEnabled = ((SessionCfgStruct_V4*)pSessionCfg)->OnTheFlyRepairEnabled;
                if(Glbv.Prv.OnTheFlyRepairEnabled)
                {
                    ResetTCIFlags(TC_OTF_OVERRIDE);
                }
                else
                {
                    SetTCIFlags(TC_OTF_OVERRIDE);
                }

                if(Glbv.Prv.rp->ReaderType == READER_TYPE_STM32)
                {
                    if((Glbv.Prv.STM32_EncryptionEnabled==TRUE) && (((SessionCfgStruct_V4*)pSessionCfg)->STM32_EncryptionEnabled==FALSE))
                    {
                        // cannot turn STM32 encryption off if currently on
                        iResult = STERR_NOT_SUPPORTED;
                    }
                    else if((Glbv.Prv.STM32_EncryptionEnabled==FALSE) && (((SessionCfgStruct_V4*)pSessionCfg)->STM32_EncryptionEnabled==TRUE))
                    {
                        // check if firmware supports STM32 encryption
                        iResult = ReadLogicalRegister(FW_REV, &fwRev, Force);
                        if(iResult == STERR_OK)
                        {
                            if(fwRev >= 0x28)
                            {
                                iResult = OnSendCmdTableUSB(ST_TOUCH_SET_ENCRYPTION, 0, NULL, 0);
                                if(iResult == STERR_OK)
                                {
                                    Glbv.Prv.STM32_EncryptionEnabled = ((SessionCfgStruct_V4*)pSessionCfg)->STM32_EncryptionEnabled;
                                }
                            }
                            else
                            {
                                iResult = STERR_NOT_SUPPORTED;
                            }
                        }
                    }
                }
                else
                {
                    // ignore for non-STM32
                    Glbv.Prv.STM32_EncryptionEnabled = ((SessionCfgStruct_V4*)pSessionCfg)->STM32_EncryptionEnabled;
                }
            }
            break;
        default:
            // unknown version
            iResult = STERR_BADPARAMETER;
    }

    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @brief The function generates a random number from the sensor: it defines 
 *        the settings for offset,gain and selbuf, then reads an image and builds
 *        a number from the single pixels of the image.
 *
 * @param apEntropy  Buffer that will contain the random number
 * @param apnSize    Size of the buffer
 *
 * @return @ref STERR_OK if the random number has been copied to the buffer
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
int STGetEntropy(void *apEntropy, int *apnSize)
// External variation, with error returns only as in API spec
{
    int iResult;

#ifdef _DEBUG
    printf("\n----------- Entering STGetEntropy -----------\n");
#endif

    iResult = STiGetEntropy(apEntropy, apnSize);

    if((iResult!=STERR_OK) && (iResult!=STERR_NODEVICE) && (iResult!=STERR_BADPARAMETER) && 
       (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        // errors must be visible but not known by the user
        iResult -= 100;
    }
    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal variation of @ref STGetEntropy, with unfiltered error returns
 *
 * @param apEntropy Buffer that will contain the random number
 * @param apnSize Size of the buffer
 *
 * @return @ref STERR_OK if the random number has been copied to the buffer
 * @return @ref STERR_NODEVICE if the TouchChip has not been open
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return @ref STERR_MEMORY_MALLOC_FAIL if memory allocation failure
 * @return (other values) Errors from functions called internally
 */
//---------------------------------------------------------------------------
int STiGetEntropy(void *apEntropy, int *apnSize)
// Internal variation, with unfiltered error returns
{
    int iResult;

    // check if STOpen has succesfully completed
    if(Glbv.Prv.PP_IntStatus != PP_Open)
    {
        return STERR_NODEVICE;
    }

    // check for NULL parameters
    if((apEntropy==NULL) || (apnSize==NULL))
    {
        return STERR_BADPARAMETER;
    }

    // check for ESD event
    iResult = Check_ESD_CommunicationError();
    if(iResult != STERR_OK)
    {
        return iResult;
    }

    // get entropy
    iResult = RandomDataGenerator(apEntropy, apnSize);

    return iResult;
}

//---------------------------------------------------------------------------
int STListReaders(ReaderListStruct *p_ReaderList)
{
    int iResult, i;

#ifdef _DEBUG
    printf("\n----------- STListReaders -----------\n");
#endif

    // check if STInitialize has not been called
    if((Glbv.Prv.PP_IntStatus!=NotInUse) && (Glbv.Prv.PP_IntStatus!=PP_Terminate))
    {
        return STERR_BAD_STATE;
    }

    // check for NULL parameters
    if(p_ReaderList == NULL)
    {
        return STERR_BADPARAMETER;
    }

    memset(p_ReaderList, 0, sizeof(ReaderListStruct));
    memset(&(Glbv.Prv.dev_info), 0, sizeof(DeviceInfoStruct));

    // get device list
    iResult = OnGetDeviceListUSB(&(Glbv.Prv.dev_info));
    if(iResult != STERR_USB_ERR_NONE)
    {
        Glbv.Prv.reader_num = 0;
        memset(&(Glbv.Prv.dev_info), 0, sizeof(DeviceInfoStruct));
    }
    else
    {
        p_ReaderList->totalReadersFound = Glbv.Prv.dev_info.NumDevices;
        for(i=0; ((i<MAX_NUM_READERS) && (i<Glbv.Prv.dev_info.NumDevices)); i++)
        {
            p_ReaderList->reader_entry[i].BUSID = Glbv.Prv.dev_info.device_entry[i].BUSID;
            memcpy(p_ReaderList->reader_entry[i].ManufactureString, Glbv.Prv.dev_info.device_entry[i].ManufactureString, MAX_DEVICE_STR_LEN);
            memcpy(p_ReaderList->reader_entry[i].DeviceString, Glbv.Prv.dev_info.device_entry[i].DeviceString, MAX_DEVICE_STR_LEN);
            memcpy(p_ReaderList->reader_entry[i].FriendlyNameString, Glbv.Prv.dev_info.device_entry[i].FriendlyNameString, MAX_DEVICE_STR_LEN);
        }
    }

    if((iResult!=STERR_OK) && (iResult!=STERR_BADPARAMETER) && (iResult!=STERR_NOT_SUPPORTED) &&
       (iResult!=STERR_BAD_STATE) && (iResult!=STERR_SENSOR_COMMUNICATION))
    {
        // errors must be visible but not known by the user
        iResult -= 100;
    }
    return( iResult );
}

//---------------------------------------------------------------------------
int STSelectReader(int readernum)
{
#ifdef _DEBUG
    printf("\n----------- STSelectReader -----------\n");
#endif

    // check if STInitialize has not been called
    if((Glbv.Prv.PP_IntStatus!=NotInUse) && (Glbv.Prv.PP_IntStatus!=PP_Terminate))
    {
        return STERR_BAD_STATE;
    }

    // validate reader number
    if((readernum<0) || (readernum>=MAX_NUM_READERS))
    {
        return STERR_BADPARAMETER;
    }

    // select reader
    if(strlen(Glbv.Prv.dev_info.device_entry[readernum].CreateFileName) == 0)
    {
        return STERR_BADPARAMETER;
    }
    Glbv.Prv.reader_num=readernum;

    return STERR_OK;
}

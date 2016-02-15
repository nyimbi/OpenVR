/**
 * @file tci.c
 * Legacy Version 2.0 Function supported by Rev. E sensor
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
/** @brief Let's use strict type checking in the example */
#define STRICT
#define GLOBAL__
#include <math.h>
#include <time.h>
#include "perfectprintint.h"
#include "ioreg.h"
#include "qualeng.h"
#include "touchip.h"
#include "crypt.h"
#include "tci.h"


//----------------------------- Local Variables -----------------------------
/**
 * @defgroup globalstructinitvars Global Structure Initialization Variables
 * Variables used to initialize the global structure GlobalVar
 * @{
 */
GlobalDef Glbv;                         ///< global structure
WindowSettings WinSett;                 ///< window settings struct
int BadRow[YMAX];                       ///< bad rows
int BadCol[XMAX];                       ///< bad columns
BadPix gBadPixelList;                   ///< bad pixels
GainOffsetStruct GainOffset;            ///< gain/offset calibration
GradientProfile gp;                     ///< gradient profile
ReaderProfile rp;                       ///< reader profile
setting TCSettingTable[MAX_SETTINGS];   ///< setting table
setting CalSettings[MAX_CAL_SETT];      ///< calibration settings for
/** @} */

/**
 * @defgroup authtable Authentify Table
 * Table with algorithm id and password id for @ref STAuthentify
 * @{
 */
struct
{
    char customer_id;
    char algoid[16];
    char pass[16];
} AuthentifyTable[MAX_ALGO_ID];
/** @} */


//--------------------------- Local Functions -------------------------------
static int Check_License(char *RetStr18, int *yes, unsigned char *SP);
static int GetAlgoIdInfo(char *algoId, char *passId, char *customer_id);
static int InitSensorVersion(int VendorCode, BYTE *SensorVersion);


//-------------------------- Function definitions ---------------------------

//---------------------------------------------------------------------------
/**
 * @brief The function initiliazes some fields of the global variable @ref GlobalDef
 */
//---------------------------------------------------------------------------
void InitGlobalVar()
{
    if(!Glbv.Prv.first_init)
    {
        Glbv.Pub.Win = &WinSett;                    ///< window settings
        Glbv.Prv.BadRow = BadRow;                   ///< bad rows
        Glbv.Prv.BadCol = BadCol;                   ///< bad columns
        Glbv.Prv.gBadPixelList = &gBadPixelList;    ///< bad pixels
        Glbv.Prv.GainOffset = &GainOffset;          ///< gain/offset calibration
        Glbv.Prv.gp = &gp;                          ///< gradient profile
        Glbv.Prv.rp = &rp;                          ///< reader profile
        Glbv.Prv.TCSettingTable = TCSettingTable;   ///< setting table
        Glbv.Prv.CalSettings = CalSettings;         ///< settings to calibrate the sensor
        Glbv.Prv.first_init = TRUE;
    }
    Glbv.Prv.PeriodWidth = 12;
    Glbv.Prv.ChargeWidth = 8;
    Glbv.Prv.ResetWidth = 7;
    Glbv.Prv.Orientation = 0;
    Glbv.Prv.isCommunicationError=FALSE;
    Glbv.Prv.inEIMOTF = FALSE;                      ///< currently processing OTF in EIM?
    Glbv.Prv.StaticFaintFingerThreshold = 50;
    Glbv.Prv.DynamicNoFingerThreshold = 30;
    Glbv.Prv.DynamicFaintFingerThreshold = 100;
    Glbv.Prv.DynamicFaintFingerTargetContrast = 125;
    Glbv.Prv.FingerTriggerThreshold = 10;
    Glbv.Prv.FingerBoxMinThreshold = 64;
    Glbv.Prv.StaticSCFaintFingerThreshold = 50;
    Glbv.Prv.DynamicSCNoFingerThreshold = 20;
    Glbv.Prv.DynamicSCFaintFingerThreshold = 50;
    Glbv.Prv.DynamicSCFaintFingerTargetContrast = 180;
    Glbv.Prv.SCFingerTriggerThreshold = 10;
    Glbv.Prv.SCFingerBoxMinThreshold = 64;
    Glbv.Prv.UseDynamicDigitalGainForFaintFinger = TRUE;
    Glbv.Prv.ApplyDigitalGain = TRUE;
    ResetTCIFlags(TC_DIG_GAIN_OVERRIDE);
    Glbv.Prv.OnTheFlyRepairEnabled = TRUE;
    ResetTCIFlags(TC_OTF_OVERRIDE);
    Glbv.Prv.STM32_EncryptionEnabled = FALSE;
}

//---------------------------------------------------------------------------
/**
 * @brief The function reads the version of the firmware and saves it in the
 * global variable FirmwareVersion - @ref fwversions.
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_FIRMWAREREV if firmware isn't supported
 * @return (other values) Error codes returned by @ref ReadLogicalRegister
 */
//---------------------------------------------------------------------------
int TCI_CheckFirmware()
{
    int iStatus, Data;

    iStatus = ReadLogicalRegister(FW_REV, &Data, Force);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    // do not support older firmware
    if(Data < VERSION_23)
    {
        return STERR_FIRMWAREREV;
    }

    Glbv.Prv.FirmwareVersion = (BYTE)Data;

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 * @brief The function checks and saves the version of the sensor.
 *
 * @return @ref STERR_OK upon success
 * @return (other values) Error codes returned by @ref ReadLogicalRegister/@ref WriteLogicalRegister
 */
//---------------------------------------------------------------------------
int TCI_CheckSensorVersion()
{
    int iStatus, Data;

    // set TestFifo to 0xff, so can be checked at grabbing for reset event
    iStatus = WriteLogicalRegister(TESTDATA, TC_STAT_NORMAL, NoForce);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    // wake up initialization sequence (power on)
    iStatus = WriteLogicalRegister(RESTART, 1, NoForce);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    iStatus = WriteLogicalRegister(STOP, 1, NoForce);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    iStatus = WriteLogicalRegister(STOP, 0, NoForce);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    iStatus = WriteLogicalRegister(LSD, 1, NoForce);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    iStatus = WriteLogicalRegister(SUPPLYOFF, 1, NoForce);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    // turn off the pixel repair function
    iStatus = WriteLogicalRegister(DISABLEUSER, 0, Force);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    // read Vendor Code 0
    iStatus = ReadLogicalRegister(VENDORCODE0, &Data, Force);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    iStatus = InitSensorVersion(Data, &Glbv.Pub.SensorVersion);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 * @brief The function assigns the version of the sensor to the global variable 
 * SensorVersion.
 *
 * @param VendorCode Vendor code read from the sensor register
 * @param SensorVersion Variable to be initialized with the sensor version - 
 *                      @ref sensorversions.  Must be pre-allocated in memory.
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_SENSOR_VERSION if the version number read from the sensor 
 * is not valid
 */
//---------------------------------------------------------------------------
static int InitSensorVersion(int VendorCode, BYTE *SensorVersion)
{
    switch(VendorCode)
    {
    case 27:
        *SensorVersion = S2C_SENSOR;
        break;
    case 28:
        *SensorVersion = S2A_SENSOR;
        break;
    case 37:
        *SensorVersion = S1A_SENSOR;
        break;
    case 38:
        *SensorVersion = S1C_SENSOR;
        break;
    case 39:
        *SensorVersion = FUTURE_SENSOR1;
        break;
    case 40:
        *SensorVersion = FUTURE_SENSOR2;
        break;
    default:
        return STERR_SENSOR_VERSION;
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 * @brief The function initialize the configuration flag according to the 
 * sensor version.
 *
 * @param SensorVersion Variable containing the sensor version - @ref sensorversions
 * @param ConfigFlag Can override pixel replacement, etc - @ref tciconfigflag.
 *                   Must be preallocated in memory.
 */
//---------------------------------------------------------------------------
void InitConfigFlag(BYTE SensorVersion, BYTE *ConfigFlag)
{
    switch(SensorVersion)
    {
    case S1A_SENSOR:
    case S2A_SENSOR:
    case FUTURE_SENSOR1:
        *ConfigFlag = 0;
        if((Glbv.Prv.rp->ReaderType == READER_TYPE_STM32) && (Glbv.Prv.rp->ReaderMode == READER_MODE_STD))
        {
            InitDigitalGainTable(GAIN_THREEANDAHALF, GAIN_ONE, 235, Glbv.Prv.D_gain);
        }
        else
        {
            InitDigitalGainTable(GAIN_ONE, GAIN_ONE, 192, Glbv.Prv.D_gain);
        }
        InitDigitalGainTable(GAIN_TWO, GAIN_ONE, 235, Glbv.Prv.FFD_gain);
        break;
    case S1C_SENSOR:
    case S2C_SENSOR:
    case FUTURE_SENSOR2:
        *ConfigFlag = TC_CFG_PIXEL_REPLACE+TC_CFG_COATED_SENSOR;
        if((Glbv.Prv.rp->ReaderType == READER_TYPE_STM32) && (Glbv.Prv.rp->ReaderMode == READER_MODE_STD))
        {
            InitDigitalGainTable(GAIN_THREEANDAHALF, GAIN_ONE, 235, Glbv.Prv.D_gain);
        }
        else
        {
            InitDigitalGainTable(GAIN_THREE, GAIN_ONE, 192, Glbv.Prv.D_gain);
        }
        InitDigitalGainTable(GAIN_TWO, GAIN_ONE, 235, Glbv.Prv.FFD_gain);
        break;
    default:
        *ConfigFlag = 0;
    }
}

//---------------------------------------------------------------------------
/**
 * @brief The measure the current used by the sensor.
 *
 * TC_IDD is 255 - ((((current) *10 Ohms)-.13V)*58.5 counts/Volt)
 *
 * @param IDD_val   Pointer to location to save the value read from the sensor
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_BAD_STATE if chip is not in imaging mode
 * @return (other values) Internal error codes returned by @ref ReadLogicalRegister/@ref WriteLogicalRegister
 */
//---------------------------------------------------------------------------
int TCI_GetIDD(BYTE *IDD_val)
{
    int iStatus, Data;

    if(Glbv.Prv.ChipState != IMAGING_MODE)
    {
        return STERR_BAD_STATE;
    }

    // first read the TC_IDD
    iStatus = ReadLogicalRegister(TC_IDD, &Data, Force);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    *IDD_val = (BYTE)Data&0x00ff;

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 * @brief The function sets up the reader latch-up current limit.
 *
 * Usually the value passed in the value just measured.  This is offset by 
 * ~10 milliamps, and set as the new threshold in the reader.
 *  TC_IDD is 255 - ((((current) *10 Ohms)-.13V)*58.5 counts/Volt)
 *
 * @param IDD_val      The threshold to set.
 *
 * @return @ref STERR_OK upon success
 * @return (other values) Internal error codes returned by @ref ReadLogicalRegister/@ref WriteLogicalRegister
 */
//---------------------------------------------------------------------------
int TCI_SetIDDLimit(BYTE IDD_val)
{
    int iStatus;

    if((Glbv.Prv.rp->ReaderMode == READER_MODE_EIM) || (Glbv.Prv.rp->ReaderMode == READER_MODE_EIM_LITE))
    {
        //Adjust the current threshold (i.e. increase) according to the decrease in power 
        //supply. In more detail: the current threshold is proportional to the inverse of 
        //the power supply. Going from 5.0V to 4.3V required to increase the current threshold 
        //of (0.7/5.0)=0.14=14%.
        IDD_val -= TC_IDD_INCREASE_EIM;

        // limit to 100 milliamps!
        IDD_val = max(IDD_val, TC_IDD_THRESH_100MA_EIM);

        // make sure it is large enough to handle the standard device current.
        IDD_val = min(IDD_val, TC_IDD_THRESH_20MA_EIM);
    }
    else
    {
        // increase by approx 10 milliamps
        // (note that lower number means higher current)
        IDD_val -= TC_IDD_INCREASE;

        // limit to 100 milliamps!
        IDD_val = max(IDD_val, TC_IDD_THRESH_100MA);

        // make sure it is large enough to handle the standard device current.
        IDD_val = min(IDD_val, TC_IDD_THRESH_20MA);
    }

    // set IDD limit
    iStatus = WriteLogicalRegister(TC_IDD, IDD_val, Force);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 * @brief The function initializes the connection with the USB driver.
 *
 * @return @ref STERR_OK upon success
 * @return (other values) Internal error codes
 */
//---------------------------------------------------------------------------
int InitializeUSB()
{
    int status = STERR_USB_ERR_NONE;

    status = OnSelectUSB(Glbv.Prv.dev_info.device_entry[Glbv.Prv.reader_num].CreateFileName, Glbv.Prv.hFingerEvent);
    if(status != STERR_USB_ERR_NONE)
    {
        memset(&(Glbv.Prv.dev_info), 0, sizeof(DeviceInfoStruct));
        if(Glbv.Prv.reader_num == 0)
        {
            status = OnSelectUSB(Glbv.Prv.dev_info.device_entry[Glbv.Prv.reader_num].CreateFileName, Glbv.Prv.hFingerEvent);
        }
    }

    return status;
}

//---------------------------------------------------------------------------
/**
 * @brief The function checks for ESD event.
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_SENSOR_COMMUNICATION if an ESD event has occurred
 */
//---------------------------------------------------------------------------
int Check_ESD_CommunicationError()
{
    int Data, iResult;

    // check if an error has already occurred
    if(Glbv.Prv.isCommunicationError)
    {
        return STERR_SENSOR_COMMUNICATION;
    }

    // check chip status (check if RESET/"light" ESD event happened)
    iResult = ReadLogicalRegister(TESTDATA, &Data, ForceSingleReg);
    if((iResult==STERR_OK) && (Data!=TC_STAT_NORMAL))
    {
        Glbv.Prv.isCommunicationError = TRUE;
        iResult = STERR_SENSOR_COMMUNICATION;
    }

    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @brief The function checks USB invalid handle/READ/WRITE errors.
 *
 * @param retval Code of the last error occurred, if any
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_SENSOR_COMMUNICATION if an ESD event has occurred
 */
//---------------------------------------------------------------------------
int Check_USB_CommunicationError(int *retval)
{
    // check USB errors (invalid handle/read/write errors)
    if((*retval==STERR_USB_ERR_HANDLE) || (*retval==STERR_USB_ERR_USBWR) || (*retval==STERR_USB_ERR_USBRD))
    {
        Glbv.Prv.isCommunicationError = TRUE;
        *retval = STERR_SENSOR_COMMUNICATION;
    }

    return *retval;
}

//---------------------------------------------------------------------------
/**
 * @brief The function sets one or more flags, depending on the input mask.
 *
 * If the bit is 1 -> the flag is set to 1, otherwise it not affected.
 *
 * @param mask Bit mask that defines which flags are set
 */
//---------------------------------------------------------------------------
void SetTCIFlags(int mask)
{
    Glbv.Prv.TC_OverRideFlags |= mask;
}

//---------------------------------------------------------------------------
/**
 * @brief The function resets one or more flags, depending on the input mask.
 *
 * If the bit is 1 -> the flag is set to 0, otherwise it not affected.
 *
 * @param mask Bit mask that defines which which flags are reset
 */
//---------------------------------------------------------------------------
void ResetTCIFlags(int mask)
{
    Glbv.Prv.TC_OverRideFlags &= ~mask;
}

//---------------------------------------------------------------------------
/**
 * @brief Gets the reader mode of the device (exported for internal use only).
 *
 * @param mode Retrieved reader mode of the device.  Only valid if return value 
 * is @ref STERR_OK.
 *
 * @return @ref STERR_OK if the reader mode was retrieved successfully
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 */
//---------------------------------------------------------------------------
int GetReaderMode(int *mode)
{
    // check if STInitialize() has succesfully completed
    if((Glbv.Prv.PP_IntStatus==NotInUse) || (Glbv.Prv.PP_IntStatus==PP_Terminate))
    {
        return STERR_NODEVICE;
    }

    // check for NULL parameters
    if(mode == NULL)
    {
        return STERR_BADPARAMETER;
    }

    *mode = (int)Glbv.Prv.rp->ReaderMode;

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 * @brief Gets the reader type of the device (exported for internal use only).
 *
 * @param type Retrieved reader type of the device.  Only valid if return value 
 * is @ref STERR_OK.
 *
 * @return @ref STERR_OK if the reader type was retrieved successfully
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 */
//---------------------------------------------------------------------------
int GetReaderType(int *type)
{
    // check if STInitialize() has succesfully completed
    if((Glbv.Prv.PP_IntStatus==NotInUse) || (Glbv.Prv.PP_IntStatus==PP_Terminate))
    {
        return STERR_NODEVICE;
    }

    // check for NULL parameters
    if(type == NULL)
    {
        return STERR_BADPARAMETER;
    }

    *type = (int)Glbv.Prv.rp->ReaderType;

    return STERR_OK;
}

//---------------------------------------------------------------------------
/** 
 * @brief Gets the sensor type of the device (exported for internal use only).
 *
 * @param type Retrieved sensor type of the device.  Only valid if return value 
 * is @ref STERR_OK.
 *
 * @return @ref STERR_OK if the sensor type was retrieved successfully
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 */
//---------------------------------------------------------------------------
int GetSensorType(int *type)
{
    // check if STInitialize() has succesfully completed
    if((Glbv.Prv.PP_IntStatus==NotInUse) || (Glbv.Prv.PP_IntStatus==PP_Terminate))
    {
        return STERR_NODEVICE;
    }

    // check for NULL parameters
    if(type == NULL)
    {
        return STERR_BADPARAMETER;
    }

    *type = (int)Glbv.Prv.rp->SensorType;

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 * @brief The function saves the current TC settings in the input variable @a settings
 *
 * @param settings Variable in which the current settings will be saved.  Must
 *                 be pre-allocated in memory.
 */
//---------------------------------------------------------------------------
void SaveSetting(int *settings)
{
    *settings = Glbv.Prv.CurrentSetting;
}

//---------------------------------------------------------------------------
/**
 * @brief The function restore the settings previously saved in @a settings
 *
 * @param settings Variable containing the saved settings
 *
 * @return @ref STERR_OK upon success
 * @return (other values) Error codes returned by other internal functions
 */
//---------------------------------------------------------------------------
int RestoreSetting(int *settings)
{
    int iStatus;

    Glbv.Prv.CurrentSetting = *settings;
    iStatus = GetSelSettings(Glbv.Prv.CurrentSetting);

    return iStatus;
}

//---------------------------------------------------------------------------
/**
 * @brief The function loads the input parameter with the current window settings.
 *
 * @param window_settings Data structure that will keep the window settings.  Must
 *                        be pre-allocated in memory.
 */
//---------------------------------------------------------------------------
void SaveWindowSettings(WindowSettings *window_settings)
{
    window_settings->StartX = GlbWin->StartX;
    window_settings->StopX  = GlbWin->StopX;
    window_settings->StartY = GlbWin->StartY;
    window_settings->StopY  = GlbWin->StopY;
    window_settings->Height = GlbWin->Height;
    window_settings->Width  = GlbWin->Width;
    window_settings->DeltaX = GlbWin->DeltaX;
    window_settings->DeltaY = GlbWin->DeltaY;
}

//---------------------------------------------------------------------------
/** 
 * @brief The function sets the window size with the values given in input
 *
 * @param window_settings Window settings to be set
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_SENSOR_COMMUNICATION if a communication error has occurred
 * @return (other values) Status returned by the functions @ref OnSendCmdTableUSB, 
 *                        @ref WriteLogicalRegister
 */
//---------------------------------------------------------------------------
int RestoreWindowSettings(WindowSettings *window_settings)
{
    int iResult;

    GlbWin->StartX  = window_settings->StartX; 
    GlbWin->StopX   = window_settings->StopX;
    GlbWin->StartY  = window_settings->StartY;
    GlbWin->StopY   = window_settings->StopY;
    GlbWin->Height  = window_settings->Height;
    GlbWin->Width   = window_settings->Width;
    GlbWin->DeltaX  = window_settings->DeltaX;
    GlbWin->DeltaY  = window_settings->DeltaY;

    iResult = TC_SetWindowSize(GlbWin->StartX, GlbWin->StartY, GlbWin->StopX, GlbWin->StopY, GlbWin->DeltaX, GlbWin->DeltaY);

    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @brief The function set the window to the maximum size
 *
 * @return @ref STERR_OK upon success
 * @return (other values) Status from internal functions
 */
//---------------------------------------------------------------------------
int SetWindowFullSize()
{
    int iResult;

    GlbWin->StartX  = 0;
    GlbWin->StopX   = (GlbWin->MaxWidth-1);
    GlbWin->StartY  = 0;
    GlbWin->StopY   = (GlbWin->MaxHeight-1);
    GlbWin->Height  = GlbWin->MaxHeight;
    GlbWin->Width   = GlbWin->MaxWidth;
    GlbWin->DeltaX  = 1;
    GlbWin->DeltaY  = 1;

    iResult = SetWindow(GlbWin->StartX, GlbWin->StartY, GlbWin->Width, GlbWin->Height, GlbWin->DeltaX, GlbWin->DeltaY);

    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @brief The function searches for the algoid string in the AuthentifyTable, and
 * returns the corrispondent client id. 
 *
 * If not found, returns -1.
 *
 * @param algoId Algoid string to be checked
 * @param passId Pass phrase to be returned
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_BADPARAMETER if algoId is not valid
 * @return @ref STERR_INVALID_LICENSE if license number is not valid
 * @return (other values) Errors returned by called functions
 */
//---------------------------------------------------------------------------
int CheckAlgoId(char *algoId, char *passId)
{
    char loc_passId[PASS_LENGTH];
    char RetStr[18];
    char client_id;
    unsigned char sp_code;
    int i, match;
    int iResult=STERR_OK;

    iResult = GetAlgoIdInfo(algoId, loc_passId, &client_id);
    if(iResult == STERR_OK)
    {
        // create random number for Check_License
        srand((unsigned)time(NULL));
        for(i=0; i<sizeof(RetStr); i++)
        {
            RetStr[i] = (char)rand();
        }

        // check if the license is valid
        iResult = Check_License(RetStr, &match, &sp_code);
        sp_code += 'A';
        if(iResult != STERR_OK)
        {
            // the test faild for invalid license number
            if(iResult == STERR_USB_ERR_USBRD)
            {
                iResult = STERR_INVALID_LICENSE;
            }
        }
        else
        {
            // the return status was ok -> check if it matched
            if(match != 0)
            {
                iResult = STERR_INVALID_LICENSE;
            }
            else
            {
                if(sp_code == PROTECTOR_ID)
                {
                    if((client_id!=PROTECTOR_ID) && (client_id!=PERFECTPRINT_ID))
                    {
                        iResult = STERR_INVALID_LICENSE;
                    }
                }
                else
                {
                    if(sp_code != client_id)
                    {
                        iResult = STERR_INVALID_LICENSE;
                    }
                }
            }
        }
    }
    else
    {
        iResult = STERR_INVALID_LICENSE;
    }

    // return the passphrase only if the test was ok
    if(iResult == STERR_OK)
    {
        // if it's Sagem -> disable finger detect
        if(client_id == SAGEM_ID)
        {
            SetTCIFlags(TC_FINGER_OVERRIDE);
        }
        memcpy(passId, loc_passId, PASS_LENGTH);
        Glbv.Prv.PP_IntStatus = PP_Authentify;  // to enable STOpen
    }
    else
    {
        memset(passId, 0xFF, PASS_LENGTH);
    }

    return iResult;
}

//---------------------------------------------------------------------------
/**
 * @brief The function initializes the AuthetifyTable, used to store the valid 
 * algorithm id and passphrase for customer identification purposes.
 */
//----------------------------------------------------------------------------
void InitAuthentify()
{
    int i=0;
    int algoid[4], pass[4];
    char cust='A';

    // Load the table using integers, equivalent to the valid strings. This is 
    // a way to avoid having strings in the source code, so that they can't be 
    // read from the executable file.
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x35463341;
    algoid[1] = 0x38353144;
    algoid[2] = 0x33424333;
    algoid[3] = 0x46303931;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x43463441;
    pass[1] = 0x42393135;
    pass[2] = 0x33454135;
    pass[3] = 0x30453136;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x41383344;
    algoid[1] = 0x34304231;
    algoid[2] = 0x37333845;
    algoid[3] = 0x41353832;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x39313841;
    pass[1] = 0x42323634;
    pass[2] = 0x39443538;
    pass[3] = 0x34464132;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x41443135;
    algoid[1] = 0x33434232;
    algoid[2] = 0x32334334;
    algoid[3] = 0x35313036;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x38443931;
    pass[1] = 0x41434639;
    pass[2] = 0x44433241;
    pass[3] = 0x34433630;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x43333635;
    algoid[1] = 0x38343444;
    algoid[2] = 0x34303844;
    algoid[3] = 0x39384534;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x34333136;
    pass[1] = 0x33443437;
    pass[2] = 0x32393042;
    pass[3] = 0x37314343;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x46414343;
    algoid[1] = 0x35413745;
    algoid[2] = 0x32443230;
    algoid[3] = 0x32314339;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x33413534;
    pass[1] = 0x35303734;
    pass[2] = 0x35323336;
    pass[3] = 0x33393543;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x43333544;
    algoid[1] = 0x41463338;
    algoid[2] = 0x39313535;
    algoid[3] = 0x42343832;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x30353235;
    pass[1] = 0x35344234;
    pass[2] = 0x46303337;
    pass[3] = 0x33314536;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x32344235;
    algoid[1] = 0x32423230;
    algoid[2] = 0x42454132;
    algoid[3] = 0x34353244;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x45393732;
    pass[1] = 0x36394234;
    pass[2] = 0x32453244;
    pass[3] = 0x35444437;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x43384132;
    algoid[1] = 0x32323744;
    algoid[2] = 0x41413338;
    algoid[3] = 0x46444235;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x41443045;
    pass[1] = 0x38414230;
    pass[2] = 0x41413441;
    pass[3] = 0x32374535;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x31444433;
    algoid[1] = 0x38354235;
    algoid[2] = 0x39364134;
    algoid[3] = 0x46304532;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x44414141;
    pass[1] = 0x43324135;
    pass[2] = 0x42323845;
    pass[3] = 0x41454230;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x34303731;
    algoid[1] = 0x34443633;
    algoid[2] = 0x39454633;
    algoid[3] = 0x41353038;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x46413537;
    pass[1] = 0x30353444;
    pass[2] = 0x38343434;
    pass[3] = 0x44343035;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x35464432;
    algoid[1] = 0x34453231;
    algoid[2] = 0x32433439;
    algoid[3] = 0x38374135;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x46414632;
    pass[1] = 0x30303235;
    pass[2] = 0x42373635;
    pass[3] = 0x37353136;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x35353141;
    algoid[1] = 0x30374130;
    algoid[2] = 0x43314439;
    algoid[3] = 0x32454239;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x41413739;
    pass[1] = 0x39443645;
    pass[2] = 0x46393846;
    pass[3] = 0x39343342;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x32354442;
    algoid[1] = 0x41314241;
    algoid[2] = 0x37323042;
    algoid[3] = 0x35424342;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x32423241;
    pass[1] = 0x44343735;
    pass[2] = 0x41454443;
    pass[3] = 0x35323441;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x43463531;
    algoid[1] = 0x38414339;
    algoid[2] = 0x34353741;
    algoid[3] = 0x31353544;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x37353631;
    pass[1] = 0x31423544;
    pass[2] = 0x38373237;
    pass[3] = 0x41363942;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x41383844;
    algoid[1] = 0x41373541;
    algoid[2] = 0x42304442;
    algoid[3] = 0x41423139;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x38374136;
    pass[1] = 0x35324642;
    pass[2] = 0x34334334;
    pass[3] = 0x44413935;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
    AuthentifyTable[i].customer_id = cust++;
    algoid[0] = 0x38453430;
    algoid[1] = 0x32373242;
    algoid[2] = 0x31323433;
    algoid[3] = 0x34384145;
    memcpy(AuthentifyTable[i].algoid, algoid, sizeof(algoid));
    pass[0] = 0x35364535;
    pass[1] = 0x41343139;
    pass[2] = 0x32314332;
    pass[3] = 0x30333841;
    memcpy(AuthentifyTable[i++].pass, pass, sizeof(pass));
}

//---------------------------------------------------------------------------
/**
 * @brief Reset the device.
 */
//----------------------------------------------------------------------------
int Reset()
{
    int iStatus;

    iStatus = WriteLogicalRegister(DISABLEUSER, 0, NoForce);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    // note that this register write will lock the firmware unless the call to 
    // STAuthentify() has already been made....
    iStatus = WriteLogicalRegister(NCORERST, 0, NoForce);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }
    iStatus = WriteLogicalRegister(NCORERST, 1, Force);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    return STERR_OK;
}

//----------------------------------------------------------------------------
/**
 * @brief Checks whether the license number corresponds to the Sensor Silicon ID. 
 *
 * The license number is sent to the ST9, which decrypts it, checks it against 
 * the Silicon ID, and either returns an encrypted version of the RetStr18, or 
 * freezes! If the license doesn't match, the ST9 will need to be unplugged and 
 * replugged. The integer "yes" will be set to 0 if the license number matches, 
 * otherwise it will be set to 1. The function works only on USB with firmware 
 * 0x17 or later.  Time: Typically takes about 16 milliseconds 
 *
 * @param RetStr18  Pointer to a random 18 byte string
 * @param yes       Pointer to an integer
 * @param SP        Pointer to the 16 byte license number string 
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_FIRMWAREREV if not supported by firmware
 * @return (other values) USB errors
 */
//---------------------------------------------------------------------------
static int Check_License(char *RetStr18, int *yes , unsigned char *SP)
{
    int retval;
    BYTE mybuffer64[64];
    BYTE testbuffer64[64];
    WORD newkey;
    //unsigned int MyColArray[3];
    //unsigned int MyRowArray[3];
    WindowSettings winsett;
    //unsigned long start, duration;

    *yes = 1;

    // test signature reading command

    SaveWindowSettings(&winsett);

    mybuffer64[0] = 2;  // set bit two to denote that license is in NVM
    memcpy(&testbuffer64[17], RetStr18, 18);

    newkey = testbuffer64[17];
    newkey <<= 8;
    newkey += testbuffer64[18];

    // encrypt the new key with preset key 15
    crypt(15, &testbuffer64[17], 2, 1);

    // encrypt the return string with the new key
    crypt(newkey, &testbuffer64[19], 16, 0);

    // copy key and string into the outgoing packet
    memcpy(&mybuffer64[18], &testbuffer64[17], 18);

    retval = OnSendCmdTableUSB((ST_TOUCH_GET_SIG|0x80), 0, mybuffer64, 50);
    if(retval == STERR_USB_ERR_USBRD)
    {
        // invalid license
        return retval;
    }
    else if(Check_USB_CommunicationError(&retval) != STERR_OK)
    {
        return retval; 
    }

    *yes = memcmp(&mybuffer64[1], &RetStr18[2], 16);

    *SP = mybuffer64[17];

    // now we need to restore the chip state.
    mybuffer64[0] = 0x1A;   // reset width
    mybuffer64[1] = Glbv.Prv.ResetWidth;
    mybuffer64[2] = 0x1B;   // charge width
    mybuffer64[3] = Glbv.Prv.ChargeWidth;
    mybuffer64[4] = 0x1C;   // period width
    mybuffer64[5] = Glbv.Prv.PeriodWidth;

    retval = USB_Write_Register_Table(3, mybuffer64);
    if(retval != STERR_OK)
    {
        return retval;
    }

    retval = RestoreWindowSettings(&winsett);
    if(retval != STERR_OK)
    {
        return retval;
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 * @brief The function searches for the algoid string in the AuthentifyTable, and
 * returns the correspondent passphrase and client id. 
 *
 * If not found, returns 1.
 *
 * @param algoId Algoid string to be checked
 * @param passId Pass phrase to be returned.  Must be pre-allocated in memory 
 *               to accomodate 8 bytes.
 * @param customer_id The id of the customer.  Must be pre-allocated in memory.
 *
 * @return 0 if algoid is a valid id -> returns passphrase and customer id
 * @return 1 if algoid is not valid
 */
//---------------------------------------------------------------------------
static int GetAlgoIdInfo(char *algoId, char *passId, char *customer_id)
{
    int i;

    for(i=0; i<MAX_ALGO_ID; i++)
    {
        if(memcmp(algoId, AuthentifyTable[i].algoid, sizeof(AuthentifyTable[i].algoid)) == 0)
        {
            break;
        }
    }

    if(i < MAX_ALGO_ID)
    {
        memcpy(passId, AuthentifyTable[i].pass, sizeof(AuthentifyTable[i].pass));
        *customer_id = AuthentifyTable[i].customer_id;
        return STERR_OK;
    }
    else
    {
        return 1;
    }
}

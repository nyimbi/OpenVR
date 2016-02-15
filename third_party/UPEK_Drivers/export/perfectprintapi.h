//==========================================================================;
/**
 * @file PerfectPrintApi.h
 * @brief PerfectPrint API error codes, structures, and external function prototypes
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * Copyright (c) 2001-  AUTHENTEC.  All Rights Reserved.
 *
 * @cond SHOWAUTHOR
 * @author Zhaoliang Wang
 * @date 14-May-2001
 * @endcond
 */
//==========================================================================;
#ifndef PERFECTPRINT_API_H__
#define PERFECTPRINT_API_H__

#ifndef DllExport
#if !defined(VERSION_LINUX)
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif
#endif

#ifndef DllImport
#if !defined(VERSION_LINUX)
#define DllImport __declspec(dllimport)
#else
#define DllImport
#endif
#endif

#if defined(VERSION_LINUX)
#include "types.h"
#endif


//-------------------------------- Constants --------------------------------
/**
 * @addtogroup errorcodes PerfectPrint Error Codes
 * @{
 */
#define STERR_OK                         0      ///< OK status
#define STERR_NOTAUTHORIZED             -1      ///< Sensor has not been authentified
#define STERR_ERROR                     -2      ///< Standard error
#define STERR_NODEVICE                  -3      ///< Sensor has not been initialized
#define STERR_DEVICEINUSE               -4      ///< Another device is currently in use
#define STERR_BADPARAMETER              -5      ///< Bad input parameter
#define STERR_NOT_SUPPORTED             -6      ///< Action is not supported
#define STERR_BAD_STATE                 -11     ///< Chip in bad state.  @ref chipstates
#define STERR_NO_LATENT_INFO            -12     ///< No latent was info found
#define STERR_BUFFER_TOO_SMALL          -20     ///< Buffer is too small
#define STERR_FIRMWAREREV               -21     ///< NVM format not valid (not supported by firmware)
#define STERR_SENSOR_VERSION            -22     ///< Version number read from the sensor is not valid.  @ref sensorversions
#define STERR_BAD_DEVICE                -23     ///< No device has been defined/open
#define STERR_NO_FINGER                 -25     ///< No finger detected on sensor
#define STERR_BAD_QUALITY_FINGER        -26     ///< Bad quality finger detected on sensor
#define STERR_FAINT_FINGER              -27     ///< Faint finger detected on sensor
#define STERR_LATENT_FINGERPRINT        -28     ///< Latent finger detected on sensor
#define STERR_SENSOR_COMMUNICATION      -29     ///< Error communicating with sensor
#define STERR_INVALID_LICENSE           -38     ///< Invalid license
#define STERR_SENSOR_IS_DIRTY           -39     ///< Sensor is dirty
#define STERR_SENSOR_NOT_CALIBRATED     -81     ///< Sensor needs to be calibrated
#define STERR_BAD_GRADIENT_DATA         -82     ///< Bad EIM gradient data detected (either missing or corrupt)
#define STERR_BEZEL_NOT_CONNECTED       -83     ///< Bezel not connected
#define STERR_GENERAL_ERROR             -100    ///< General error
/** @} */

/**
 * @defgroup sensorversions Sensor Versions
 * @{
 */
#define G_SENSOR                        3       ///< REVG sensor
#define S1A_SENSOR                      4       ///< TCS1A sensor
#define S2A_SENSOR                      5       ///< TCS2A sensor
#define S1C_SENSOR                      6       ///< TCS1C sensor
#define S2C_SENSOR                      7       ///< TCS2C sensor
/** @} */

/**
 * @defgroup maxwindow Maximum Window Sizes
 * @{
 */
#define XMAX                            256     ///< Maximum width of sensor
#define YMAX                            360     ///< Maximum height of sensor
/** @} */

#define MAX_BAD_PIX                     50      ///< Maximum number of bad pixels in @ref BadPix structure
#define MAX_NUM_READERS                 5       ///< Maximum number of readers to be supported
#define MAX_DEVICE_STR_LEN              50      ///< Maximum length of reader information strings

/**
 * @defgroup chipstates Chip States
 *
 * The state machine for these power modes is described below; any of the two 
 * states can be chosen at any time. No specific procedure is required to switch among 
 * the states, but some delay has to be respected. Please note that NO DELAY means that 
 * upon assertion of the relative command, no delay is necessary to change state.
 *
 * @image html chipstates.jpg
 * @image latex chipstates.eps
 * @{
 */
#define SLEEP_MODE                      1       ///< Sleep mode for sensor (all of the analog part is shut down and the chip draws the minimum current)
#define IMAGING_MODE                    2       ///< Imaging mode for sensor (all of the block circuits are powered on)
/** @} */

/**
 * @addtogroup oemfieldflags OEM Field Flags
 *
 * 64-byte OEM fields in NVM
 * @{
 */
#define OEM_FLAG_APPFIELD               0x01    ///< References the User part of the OEM
#define OEM_FLAG_OEMFIELD               0x02    ///< References the OEM part of the OEM
/** @} */

/** 
 * @defgroup latentmodes Latent Modes
 *
 * @{
 */
#define LATENT_CHECK_ONLY               0x00000001  ///< Only check to see if is latent fingerprint
#define LATENT_UPDATE_ONLY              0x00000002  ///< Only save latent template in NVM
/** @} */

/** 
 * @defgroup sessioncfgversions Session Configuration Versions
 *
 * @{
 */
#define SESSION_CFG_V1                  0x00000001  ///< Session configuration version 1
#define SESSION_CFG_V2                  0x00000002  ///< Session configuration version 2
#define SESSION_CFG_V3                  0x00000003  ///< Session configuration version 3
#define SESSION_CFG_V4                  0x00000004  ///< Session configuration version 4
#define SESSION_CFG_DEFAULT         SESSION_CFG_V4  ///< Use session configuration version 4 as default
/** @} */


//-------------------------------- Structures -------------------------------

/// Type definition for @ref STOpen function
typedef struct
{
    BYTE SensorVersion;             ///< Sensor version code - @ref sensorversions
    unsigned short ImageWidth;      ///< Sensor image width
    unsigned short ImageHeight;     ///< Sensor image height
} SensorDescr;


/// Structure for storing pixels coordinates (x,y)
typedef struct
{
    USHORT xcor;                    ///< x-coordinate of pixel
    USHORT ycor;                    ///< y-coordinate of pixel
} pixel;

/// Stores number and list of bad pixels detected on sensor
typedef struct
{
    USHORT NumBadPixels;            ///< Number of bad pixels in this structure
    pixel BadPixLst[MAX_BAD_PIX];   ///< List of bad pixels
} BadPix;

/// Stores gain/offset calibration data acquired from @ref STSensorTest
typedef struct
{
    double gain_ro;                 ///< Readout gain
    double offset_ro;               ///< Readout offset
    double gain_pix;                ///< Pixel gain
    double offset_pix;              ///< Pixel offset
} GainOffsetStruct;

/// Structure for input parameter to @ref STSensorTest
typedef struct
{
    int BadRow[YMAX];               ///< List of sensor's bad rows
    int BadCol[XMAX];               ///< List of sensor's bad columns
    BadPix BadPixels;               ///< List of sensor's bad pixels
    GainOffsetStruct GainOffset;    ///< Gain/offset calibration for sensor
    int Gradient[YMAX];             ///< Gradient profile for sensor
} RepairDataStruct;

/// Information for a reader
typedef struct
{
    int BUSID;                                      ///< Bus ID
    char ManufactureString[MAX_DEVICE_STR_LEN];     ///< Manufacture string
    char DeviceString[MAX_DEVICE_STR_LEN];          ///< Device string
    char FriendlyNameString[MAX_DEVICE_STR_LEN];    ///< Friendly name string
} ReaderListEntry;

/// Structure for input parameter to @ref STListReaders
typedef struct
{
    int totalReadersFound;                          ///< Total number of readers detected
    ReaderListEntry reader_entry[MAX_NUM_READERS];  ///< List of readers detected
} ReaderListStruct;

/// Structure for session configuration version 1
typedef struct
{
    unsigned int FaintFinger_MinContrast;           ///< Fingers with contrast below this value are considered "faint/weak"
    unsigned int FaintFinger_BkgDiffTolerance;      ///< The tolerated difference amount between a background pixel and finger pixel
    BOOL         FaintFinger_ApplyDigitalGain;      ///< Apply digital gain compensation to faint finger image grabs
    unsigned int LatentMatch_MaxXTranslation;       ///< The max tolerated X translation between latent templates
    unsigned int LatentMatch_MaxYTranslation;       ///< The max tolerated Y translation between latent templates
    unsigned int LatentMatch_MaxRotation;           ///< The max tolerated rotation between latent templates
} SessionCfgStruct_V1;

/// Structure for session configuration version 2
typedef struct
{
    unsigned int FaintFinger_StaticFaintContrastTH;         ///< Fingers with contrast below this value are considered "faint/weak" (static digital gain)
    unsigned int FaintFinger_DynamicNoFingerContrastTH;     ///< Fingers with contrast below this value are considered as non-existent (dynamic digital gain)
    unsigned int FaintFinger_DynamicFaintContrastTH;        ///< Fingers with contrast below this value are considered "faint/weak" (dynamic digital gain)
    unsigned int FaintFinger_DynamicTargetContrast;         ///< Target contrast for faint finger image grabs after digital gain is applied (dynamic digital gain)
    unsigned int FaintFinger_FingerTriggerTH;               ///< The tolerated difference amount between a background pixel and finger pixel
    unsigned int FaintFinger_BoxMinTH;                      ///< Image contrast is computed inside a fingerprint-contained box of this mimimum size
    BOOL         FaintFinger_UseDynamicDigitalGain;         ///< Use dynamic digital gain compensation rather than static with faint finger image grabs
    BOOL         FaintFinger_ApplyDigitalGain;              ///< Apply digital gain compensation to faint finger image grabs
    unsigned int LatentMatch_MaxXTranslation;               ///< The max tolerated X translation between latent templates
    unsigned int LatentMatch_MaxYTranslation;               ///< The max tolerated Y translation between latent templates
    unsigned int LatentMatch_MaxRotation;                   ///< The max tolerated rotation between latent templates
    BOOL         OnTheFlyRepairEnabled;                     ///< On-the-fly repair enabled during image grabbing
    BOOL         STM32_EncryptionEnabled;                   ///< Encryption enabled for NVM access and image grabbing for STM32 devices
                                                            ///< Note: Encryption for STM32 cannot be disabled if currently enabled in 
                                                            ///< session (i.e. value SessionCfgStruct_V2.STM32_EncryptionEnabled cannot 
                                                            ///< be set to FALSE if it is currently TRUE).
} SessionCfgStruct_V2;

/// Structure for session configuration version 3
typedef struct
{
    unsigned int FaintFinger_StaticFaintContrastTH;         ///< Fingers with contrast below this value are considered "faint/weak" for normal sensors (static digital gain)
    unsigned int FaintFinger_DynamicNoFingerContrastTH;     ///< Fingers with contrast below this value are considered as non-existent for normal sensors (dynamic digital gain)
    unsigned int FaintFinger_DynamicFaintContrastTH;        ///< Fingers with contrast below this value are considered "faint/weak" for normal sensors (dynamic digital gain)
    unsigned int FaintFinger_DynamicTargetContrast;         ///< Target contrast for faint finger image grabs after digital gain is applied for normal sensors (dynamic digital gain)
    unsigned int FaintFinger_FingerTriggerTH;               ///< The tolerated difference amount between a background pixel and finger pixel for normal sensors
    unsigned int FaintFinger_BoxMinTH;                      ///< Image contrast is computed inside a fingerprint-contained box of this mimimum size for normal sensors
    unsigned int FaintFinger_StaticSCFaintContrastTH;       ///< Fingers with contrast below this value are considered "faint/weak" for steelcoat sensors (static digital gain)
    unsigned int FaintFinger_DynamicSCNoFingerContrastTH;   ///< Fingers with contrast below this value are considered as non-existent for steelcoat sensors (dynamic digital gain)
    unsigned int FaintFinger_DynamicSCFaintContrastTH;      ///< Fingers with contrast below this value are considered "faint/weak" for steelcoat sensors (dynamic digital gain)
    unsigned int FaintFinger_DynamicSCTargetContrast;       ///< Target contrast for faint finger image grabs after digital gain is applied for steelcoat sensors (dynamic digital gain)
    unsigned int FaintFinger_SCFingerTriggerTH;             ///< The tolerated difference amount between a background pixel and finger pixel for steelcoat sensors
    unsigned int FaintFinger_SCBoxMinTH;                    ///< Image contrast is computed inside a fingerprint-contained box of this mimimum size for steelcoat sensors
    BOOL         FaintFinger_UseDynamicDigitalGain;         ///< Use dynamic digital gain compensation rather than static with faint finger image grabs
    unsigned int LatentMatch_MaxXTranslation;               ///< The max tolerated X translation between latent templates
    unsigned int LatentMatch_MaxYTranslation;               ///< The max tolerated Y translation between latent templates
    unsigned int LatentMatch_MaxRotation;                   ///< The max tolerated rotation between latent templates
    BOOL         ApplyDigitalGain;                          ///< Apply digital gain compensation to steelcoat sensors and faint finger image grabs
    BOOL         OnTheFlyRepairEnabled;                     ///< On-the-fly repair enabled during image grabbing
    BOOL         STM32_EncryptionEnabled;                   ///< Encryption enabled for NVM access and image grabbing for STM32 devices
                                                            ///< Note: Encryption for STM32 cannot be disabled if currently enabled in 
                                                            ///< session (i.e. value SessionCfgStruct_V3.STM32_EncryptionEnabled cannot 
                                                            ///< be set to FALSE if it is currently TRUE).
} SessionCfgStruct_V3;

/// Structure for session configuration version 4
typedef struct
{
    unsigned int FaintFinger_StaticFaintContrastTH;         ///< Fingers with contrast below this value are considered "faint/weak" for normal sensors (static digital gain)
    unsigned int FaintFinger_DynamicNoFingerContrastTH;     ///< Fingers with contrast below this value are considered as non-existent for normal sensors (dynamic digital gain)
    unsigned int FaintFinger_DynamicFaintContrastTH;        ///< Fingers with contrast below this value are considered "faint/weak" for normal sensors (dynamic digital gain)
    unsigned int FaintFinger_DynamicTargetContrast;         ///< Target contrast for faint finger image grabs after digital gain is applied for normal sensors (dynamic digital gain)
    unsigned int FaintFinger_FingerTriggerTH;               ///< The tolerated difference amount between a background pixel and finger pixel for normal sensors
    unsigned int FaintFinger_BoxMinTH;                      ///< Image contrast is computed inside a fingerprint-contained box of this mimimum size for normal sensors
    unsigned int FaintFinger_StaticSCFaintContrastTH;       ///< Fingers with contrast below this value are considered "faint/weak" for steelcoat sensors (static digital gain)
    unsigned int FaintFinger_DynamicSCNoFingerContrastTH;   ///< Fingers with contrast below this value are considered as non-existent for steelcoat sensors (dynamic digital gain)
    unsigned int FaintFinger_DynamicSCFaintContrastTH;      ///< Fingers with contrast below this value are considered "faint/weak" for steelcoat sensors (dynamic digital gain)
    unsigned int FaintFinger_DynamicSCTargetContrast;       ///< Target contrast for faint finger image grabs after digital gain is applied for steelcoat sensors (dynamic digital gain)
    unsigned int FaintFinger_SCFingerTriggerTH;             ///< The tolerated difference amount between a background pixel and finger pixel for steelcoat sensors
    unsigned int FaintFinger_SCBoxMinTH;                    ///< Image contrast is computed inside a fingerprint-contained box of this mimimum size for steelcoat sensors
    BOOL         FaintFinger_UseDynamicDigitalGain;         ///< Use dynamic digital gain compensation rather than static with faint finger image grabs
    BOOL         ApplyDigitalGain;                          ///< Apply digital gain compensation to steelcoat sensors and faint finger image grabs
    BOOL         OnTheFlyRepairEnabled;                     ///< On-the-fly repair enabled during image grabbing
    BOOL         STM32_EncryptionEnabled;                   ///< Encryption enabled for NVM access and image grabbing for STM32 devices
                                                            ///< Note: Encryption for STM32 cannot be disabled if currently enabled in 
                                                            ///< session (i.e. value SessionCfgStruct_V4.STM32_EncryptionEnabled cannot 
                                                            ///< be set to FALSE if it is currently TRUE).
} SessionCfgStruct_V4;

typedef SessionCfgStruct_V4 SessionCfgStruct;       ///< Use session configuration version 4 as default


//--------------------------- Function prototypes ---------------------------

/* Assume C declarations for C++ callers */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes communication channel with the sensor
 *
 * hFingerLeft is not used in version 1.1 and earlier it's just 
 * for future compatibility.  It should be set to @c NULL in version 1.1
 * and earlier.
 *
 * @param hFingerPresent    For future compatibility. Use @c NULL.
 * @param hFingerLeft       For future compatibility. Use @c NULL.
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_DEVICEINUSE if another device is in use
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors returned by internal functions
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STInitialize(HANDLE hFingerPresent, HANDLE hFingerLeft);

/**
 * @brief Opens the sensor
 *
 * @param SensorInfo    Characteristics about the sensor opened (returned)
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NOTAUTHORIZED if sensor has not been authentified
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_DEVICEINUSE if another device is in use
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_NOT_SUPPORTED if device is not supported
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors returned by internal functions
 * @return @ref STERR_SENSOR_NOT_CALIBRATED if sensor needs to be calibrated first
 * @return @ref STERR_BAD_GRADIENT_DATA if gradient data in NVM, Windows registry or 
 * external file is missing/corrupt
 * @return @ref STERR_BEZEL_NOT_CONNECTED if the bezel is not connected
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STOpen(SensorDescr *SensorInfo);

/**
 * @brief Checks the validity of the @a AlgoID string, and returns the correspondent pass 
 * phrase. 
 *
 * @param AlgoID        Algoid string to be validated
 * @param PassPhrase    Pass phrase (returned)
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_DEVICEINUSE if another device is in use
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return @ref STERR_INVALID_LICENSE if license is invalid
 * @return (other values) Errors returned by internal functions
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STAuthentify(unsigned char *AlgoID, unsigned char *PassPhrase);

/**
 * @brief Resets and shuts down the connection to the sensor
 *
 * Must be called once for each call to @ref STOpen
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors returned by internal functions
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STClose();

/**
 * @brief Stops the use of the module
 *
 * Must be called once for each call to @ref STInitialize
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return (other values) Errors returned by internal functions
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STTerminate();

/**
 * @brief Sets the grabbing window size of the sensor
 *
 * @param X0    X coordinate of the window starting point {0:248}
 * @param Y0    Y coordinate of the window starting point {0:356}
 * @param X     Width of the window {8:XMAX}
 * @param Y     Height of the Window {8:YMAX}
 * @param IncX  Sub sample on the X axes {1,2,4,8,16}
 * @param IncY  Sub sample on the Y axes {1,2,4,8,16}
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors returned by internal functions
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STSetWindow(int X0, int Y0, int X, int Y, int IncX, int IncY);

/**
 * @brief Captures raw fingerprint data according to the window size set by 
 * @ref STSetWindow function
 *
 * @param Buffer        Buffer in which to save the grabbed image. Must be 
 *                      pre-allocated in memory to accomodate selected image window.
 * @param BufferSize    The size (in bytes) of the buffer
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_BAD_STATE	if chip in bad state
 * @return @ref STERR_BUFFER_TOO_SMALL if image buffer cannot hold grabbed image
 * @return @ref STERR_NO_FINGER if no finger detected on sensor
 * @return @ref STERR_BAD_QUALITY_FINGER if a bad quality finger is detected on sensor
 * @return @ref STERR_FAINT_FINGER if a faint finger is detected on sensor
 * @return @ref STERR_LATENT_FINGERPRINT if a latent fingerprint was detected
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors returned by internal functions
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STGrab(unsigned char *Buffer, unsigned long BufferSize);

/**
 * @brief Captures raw fingerprint data using the best setting.
 *
 * @param Buffer        Buffer in which to save the grabbed image. Must be 
 *                      pre-allocated in memory to accomodate selected image window.
 * @param BufferSize    The size (in bytes) of the buffer
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_BAD_STATE if chip in bad state
 * @return @ref STERR_BUFFER_TOO_SMALL if image buffer cannot hold grabbed image
 * @return @ref STERR_NO_FINGER if no finger detected on sensor
 * @return @ref STERR_BAD_QUALITY_FINGER if a bad quality finger is detected on sensor
 * @return @ref STERR_FAINT_FINGER if a faint finger is detected on sensor
 * @return @ref STERR_LATENT_FINGERPRINT if a latent fingerprint was detected
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors returned by internal functions
 */  
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STGrabBestSett(unsigned char *Buffer, unsigned long BufferSize);

/**
 * @brief Captures raw fingerprint data using a pre select setting
 *
 * Using this function will change the sensor setting for all future 
 * operations
 *
 * @param Buffer        Buffer in which to save the grabbed image. Must be 
 *                      pre-allocated in memory to accomodate selected image window.
 * @param BufferSize    The size (in bytes) of the buffer
 * @param SelSett       The setting at which to set the sensor
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_BAD_STATE if chip in bad state
 * @return @ref STERR_BUFFER_TOO_SMALL if image buffer cannot hold grabbed image
 * @return @ref STERR_NO_FINGER if no finger detected on sensor
 * @return @ref STERR_BAD_QUALITY_FINGER if a bad quality finger is detected on sensor
 * @return @ref STERR_FAINT_FINGER if a faint finger is detected on sensor
 * @return @ref STERR_LATENT_FINGERPRINT if a latent fingerprint was detected
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors returned by internal functions
 */  
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STGrabSelSett(unsigned char *Buffer, unsigned long BufferSize, int SelSett);

/**
 * @brief This function returns a buffer containing multiple small images, each 
 * one of 32x32 bytes and with a different setting value.
 *
 * The small images are captured right in the center of the sensor.
 *
 * @param Buffer        Buffer in which to save the data. Must be pre-allocated in memory
 *                      to accomodate the total number of supported settings.
 * @param BufferSize    The size (in bytes) of the buffer = 32*32*TOTALSETTINGS, where 
 *                      TOTALSETTINGS can be found by using the @ref STGetTotalSettings 
 *                      function to get the total number of supported settings for the sensor.
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_BAD_STATE if chip in bad state
 * @return @ref STERR_BUFFER_TOO_SMALL if image buffer cannot hold grabbed image
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors returned by internal functions
 */  
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STGrabMosaic(unsigned char *Buffer, unsigned long BufferSize);

/**
 * @brief Converts a 508 DPI image to its 500 DPI equivalent
 *
 * @param inBuffer  Buffer containing a 508 DPI image retrieved by one of the many grabbing functions.
 * @param inWidth   The width of the input buffer
 * @param inHeight  The height of the input buffer
 * @param outBuffer Buffer containing the coverted 500 DPI image (returned)
 * @param outWidth  The width of the output buffer (returned)
 * @param outHeight The height of the output buffer (returned)
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return (other values) Errors returned by internal functions
 */  
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STImage508DPITo500DPI(unsigned char *inBuffer, int inWidth, int inHeight, unsigned char *outBuffer, int *outWidth, int *outHeight);

/**
 * @brief Converts a full-sized image to its ANSI 381 equivalent
 *
 * @param inBuffer      Buffer containing a full-sized image retrieved by one of the
 *                      many grabbing functions
 * @param inWidth       The width of the input image buffer
 * @param inHeight      The height of the input image buffer
 * @param outBuffer     Buffer in which to save the coverted ANSI 381 image. Must be 
 *                      pre-allocated in memory to accomodate the size of the input 
 *                      image buffer plus an additional 50 bytes for the ANSI 381 header
 * @param outBufferSize The size (in bytes) of the output buffer.  Must be at least
 *                      the size of the input image buffer plus an additional 50 bytes 
 *                      for the ANSI 381 header
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return (other values) Errors returned by internal functions
 */  
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STImageToANSI381(unsigned char *inBuffer, int inWidth, int inHeight, unsigned char *outBuffer, unsigned long outBufferSize);

/**
 * @brief Set the sensor in the desired setting
 *
 * Using this function will change the sensor setting for all future operations
 *
 * @param data  The setting at which to set the sensor
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors returned by internal functions
 */  
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STInitSensorPar(int data);

/**
 * @brief Selects the best settings parameters to be used to grab the finger on the 
 * sensor prior to the capture phase.
 *
 * The finger must be on the sensor to correctly initialize the settings.
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BAD_STATE if chip in bad state
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors returned by internal functions
 */  
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STGetBestSett();

/**
 * @brief This function returns the revision information.
 *
 * @a i_pTCIRev[0] corresponds to the Release of PerfectPrint
 * @a i_pTCIRev[1] corresponds to the Version of PerfectPrint
 * @a i_pTCIRev[2] corresponds to the Sub-version of PerfectPrint
 * @a i_pTCIRev[3] corresponds to the Sub-Sub-version of PerfectPrint
 *
 * @param i_pTCIRev     An array of size 4.  Must be pre-allocated in memory.
 * @param i_szDesc      Contains the entire description
 * @param DescBufSize   Size of the @a i_szDesc buffer which should be big enough to
 *                      keep the revision information (>80)
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_BUFFER_TOO_SMALL if description buffer @a i_szDesc is too small
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STGetTCIRevision(char *i_pTCIRev, char *i_szDesc, int DescBufSize);

/**
 * @brief Restores initial device settings and puts the chip in @c IMAGING_MODE.
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors from functions called internally
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STResetDevice();

/**
 * @brief Retrieves the current sensor setting
 *
 * @return The current setting of the sensor
 * @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STGetCurrentSetting();

/**
 * @brief Retrieves the total number of settings supported by the sensor
 *
 * @return The total number of settings supported by the sensor
 * @return STERR_NODEVICE if there is no device connected
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STGetTotalSettings();

/**
 * @brief Retrieves the current chip state
 *
 * @see chipstates
 *
 * @return The current state of the chip
 * @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STGetChipState();

/**
 * @brief Sets the chip in the specified state
 *
 * @param data  The state to which to set the chip - @ref chipstates
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors returned by internal functions
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STSetChipState(int data);

/**
 * @brief The function generates a random number from the sensor: it defines 
 * the settings for offset, gain and selbuf, then reads an image and builds
 * a number from the single pixels of the image.
 *
 * @param apEntropy Buffer large enough to hold random data
 * @param apnSize   In input, it specifies the requested data size in bytes.
 *                  In output, it specifies the size of the actual data generated by
 *                  the function, in bytes (<= than requested data size).
 *
 * @return @ref STERR_OK if the random number has been copied to the buffer
 * @return @ref STERR_NODEVICE if the TouchChip has not been open
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors from functions called internally
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STGetEntropy(void *apEntropy, int *apnSize);

/**
 * @brief This function lists all the readers found.
 *
 * The call to this function is optional for the application. It must be called 
 * before @ref STInitialize, or after @ref STTerminate. If the application does not 
 * select a specific reader, the @ref STInitialize function opens the first TouchChip 
 * reader that it finds. PerfectPrint does not support simultaneous access to more than 
 * one reader.
 *
 * Note that due to OS limitations, the strings are not read under Windows 98 or 98 SE.
 * The @a FriendlyName string is not supported under any OS.  Also note that attachment
 * of more than one reader is not supported on Windows NT.
 *
 * @param p_ReaderList  Structure to contain information on each reader found
 *
 * @see STSelectReader
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_NOT_SUPPORTED if the action is not supported
 * @return @ref STERR_BAD_STATE if the sensor is not in the correct state
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors from functions called internally
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STListReaders(ReaderListStruct *p_ReaderList);

/**
 * @brief This function selects the specified reader by @a readernum.
 *
 * This function has to be called before @ref STInitialize, or after @ref STTerminate.
 * After a call to @ref STListReaders, this function is used to select from among the
 * detected readers. The selected reader is used for all subsequent PerfectPrint
 * operations. If an error occurs during @ref STInitialize, the reader list is reset,
 * and the default (first) reader is selected.
 *
 * @param readernum The reader number to be selected (zero based)
 *
 * @see STListReaders
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_BAD_STATE if the sensor is not in the correct state
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return (other values) Errors from functions called internally
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STSelectReader(int readernum);

/**
 * @brief This function reads the value of Application register.
 *
 * With certain ST9 firmware versions, this function allows detection 
 * of the LED interface.
 *
 * @param Data  Address of an integer variable to save the value read.
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_BAD_DEVICE if no device has been defined/open
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors from functions called internally
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STReadAppReg(int *Data);

/**
 * @brief This function writes the Application register.
 *
 * With certain ST9 firmware versions, this function allows detection 
 * of the LED interface.
 *
 * @param Data  The value to be written
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 * @return @ref STERR_BAD_DEVICE if no device has been defined/open
 * @return @ref STERR_SENSOR_COMMUNICATION if there was an error commmunicating with the sensor
 * @return (other values) Errors from functions called internally
 */
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STWriteAppReg(int Data);

/**
 * @brief Captures raw fingerprint data with no finger detection according to the
 * window size set by @ref STSetWindow function
 *
 * Bypasses the finger detection, for use with navigation functions.
 *
 * @param Buffer        Buffer in which to save the grabbed image.  Must be
 *                      pre-allocated in memory.
 * @param BufferSize    The size of the image buffer
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_BAD_STATE if chip in bad state
 * @return @ref STERR_BUFFER_TOO_SMALL if image buffer cannot hold grabbed image
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return (other values) Errors returned by internal functions
 */  
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STGrabNFD(unsigned char *Buffer, unsigned long BufferSize);

/**
 * @brief Captures raw fingerprint data using the best setting with no finger detection
 *
 * Best settings grab with on-the-fly repair, but no finger detect.
 *
 * @param Buffer        Buffer in which to save the grabbed image. Must be
 *                      pre-allocated in memory.
 * @param BufferSize    The size of the image buffer
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_BAD_STATE if chip in bad state
 * @return @ref STERR_BUFFER_TOO_SMALL if image buffer cannot hold grabbed image
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return (other values) Errors returned by internal functions
 */  
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STGrabBestSettNFD(unsigned char *Buffer, unsigned long BufferSize);

/**
 * @brief Retrieves the configuration values for the current session
 *
 * @param SessionVersion    The session configuration version to retrieve - @ref sessioncfgversions
 * @param pSessionCfg       Session configuration structure in which to save the retrieved data - @ref SessionCfgStruct.
 *                          Must be pre-allocated in memory.
 * @param SessionCfgSize    Size of pSessionCfg in bytes
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 */  
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STGetSessionCfg(unsigned int SessionVersion, void *pSessionCfg, unsigned int SessionCfgSize);

/**
 * @brief Configures the current session to the given configuration values
 *
 * @param SessionVersion    The session configuration version to use - @ref sessioncfgversions
 * @param pSessionCfg       Session configuration structure containing the desired session configuration - @ref SessionCfgStruct.  
 * @param SessionCfgSize    Size of pSessionCfg in bytes
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if no device is connected
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_NOT_SUPPORTED if device is not supported
 * @return @ref STERR_BAD_DEVICE if no device has been defined/open
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 */  
#if DLL_EXPORT_FEATURE
DllExport
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall STSetSessionCfg(unsigned int SessionVersion, void *pSessionCfg, unsigned int SessionCfgSize);

#ifdef __cplusplus
}
#endif

#endif  // #ifndef PERFECTPRINT_API_H__

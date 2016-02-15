//==========================================================================;
/**
 * @file PerfectPrintInt.h
 * @brief PerfectPrint ST-Internal errors, structures, functions
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
//==========================================================================;
#ifndef PERFECTPRINT_INT_H__
#define PERFECTPRINT_INT_H__

// Define external types for antilatent and biolib
#define AL_USE_EXTERNAL_TYPES
#define BL_USE_EXTERNAL_TYPES
typedef unsigned char       uint8;
typedef signed   char       sint8;
typedef unsigned char       bool8;
typedef unsigned short      uint16;
typedef signed   short      sint16;
typedef unsigned short      bool16;
typedef float               float32;
typedef unsigned int        uint32;
typedef signed   int        sint32;
typedef unsigned int        bool32;
typedef unsigned long long  uint64;
typedef long long           sint64;
typedef double              float64;

#include <assert.h>                     // for asertion testing
#include <stdlib.h>                     // for atoi() prototype
#include <memory.h>                     // for _fmemset()
#include <stdio.h>                      // for sprintf()
#include <ctype.h>                      // for ascii test macros
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "reg_map.h"                    // register map
#include "resource.h"
#include "build_features.h"             // features for building PerfectPrint
#include "ppfeatures.h"                 // features specific to the project
#include "perfectprintapi.h"            // external constant and function definitions
#include "settings.h"                   // settings table stuff
#include "deviceList.h"

#define DECLARE_FUNCTION(ReturnType, Name, Params) ReturnType Name Params
#define DEFINE_FUNCTION(ReturnType, Name, Params) ReturnType Name Params


//-------------------------------- Constants --------------------------------
/**
 * @addtogroup errorcodes PerfectPrint Error Codes
 * @{
 */
#define STERR_OFFSET_TOO_LOW            -50     ///< Offset is too low
#define STERR_CHARGE_TOO_LOW            -51     ///< Charge is too low
#define STERR_TOO_MANY_BAD_LINES        -52     ///< Too many bad lines detected on sensor
#define STERR_TOO_MANY_BAD_PIXELS       -53     ///< Too many bad pixels detected on sensor
#define STERR_CHIP_IS_UNREPAIRABLE      -54     ///< Chip is unrepairable
#define STERR_RECORD_NOT_FOUND          -55     ///< Record not found
#define STERR_INVALID_VALUE             -56     ///< Invalid value
#define STERR_GAIN_OFFSET               -57     ///< Gain/Offset error
#define STERR_READ_REPAIR_INFO          -58     ///< Cannot read repair info

// Errors returned by NVM functions
#define STERR_NVM_READ                  -71     ///< Error in reading from NVM
#define STERR_NVM_WRITE                 -72     ///< Error in writing to NVM

// Errors returned by Memory functions
#define STERR_MEMORY_MALLOC_FAIL        -73     ///< Memory allocation failure

// Errors returned by Veridicom functions
#define STERR_CHECK_LATENT_FAIL         -75     ///< Latent finger detection failure

// Errors returned by Windows Registry functions
#define STERR_WIN_REG_FAIL              -76     ///< Windows registry failure

// Errors returned by File I/O functions
#define STERR_FILE_IN_OUT_FAIL          -77     ///< File I/O failure

// Errors returned by the USB driver: do not change their value
#define STERR_USB_ERR_NONE              0       ///< no error
#define STERR_USB_ERR_DRVOPEN           -101    ///< unable to open USB driver
#define STERR_USB_ERR_USBRD             -102    ///< USB failure during read
#define STERR_USB_ERR_USBWR             -103    ///< USB failure during write
#define STERR_USB_ERR_BUFFER            -104    ///< image size mismatch between chip and application
#define STERR_USB_ERR_IMAGE             -105    ///< incomplete image
#define STERR_USB_ERR_RANGE             -106    ///< register value out of range
#define STERR_USB_ERR_INVALID           -107    ///< target unable to read register value
#define STERR_USB_ERR_CRYPT             -108    ///< target unable to read register value
#define STERR_USB_ERR_SC_RET            -109    ///< unexpected smartcard return value
#define STERR_USB_ERR_SC_NO_CARD        -110    ///< SmartCard did not answer reset
#define STERR_USB_ERR_HANDLE            -111    ///< USB invalid handle
#define STERR_USB_HASH_MISMATCH         -201
#define STERR_USB_INVALID_PACKET        -202
/** @} */

/**
 * @defgroup fwversions Firmware Versions
 * @{
 */
#define VERSION_23                      0x17
#define VERSION_24                      0x18
/** @} */

/**
 * @defgroup futuresensorversions Future Sensor Versions
 * @{
 */
#define FUTURE_SENSOR1                  8       ///< TCS1A-like behavior for vendor code 39
#define FUTURE_SENSOR2                  9       ///< TCS1C-like behavior for vendor code 40
#define FUTURE_SENSOR3                  10      ///< Config-defined behavior...no gain/offset limits
#define FUTURE_SENSOR4                  11      ///< Config-defined behavior...no gain/offset limits
#define FUTURE_SENSOR5                  12      ///< Config-defined behavior...no gain/offset limits
/** @} */

/**
 * @defgroup devicetypes Device Types
 * @{
 */
#define ST_NO_DEVICE                    0
#define ST_USB                          1
/** @} */

/**
 * @defgroup testdata Test Data Values
 *
 * Values used in the TestData register to determine status of TouchChip
 * @{
 */
#define TC_STAT_NORMAL                  0xFF
#define TC_STAT_SUSPENDED               0xFE
#define TC_STAT_UNLATCHED               0xFD
#define TC_STAT_DEFAULT                 0x00
/** @} */

/**
 * @defgroup usbcommandtype USB Command Type Constants
 * @{
 */
#define ST_TOUCH_GET_REG_BLOCK          2       ///< Read a set of TC registers
#define ST_TOUCH_SET_REG_BLOCK          3       ///< Write a set of TC registers
#define ST_TOUCH_GET_SIG                4       ///< Read the TC signature
#define ST_TOUCH_LOAD_TABLE             7       ///< Settings table
#define ST_TOUCH_SET_WINDOW             8       ///< Set window command
#define ST_TOUCH_SET_REPAIR             9       ///< Repair info
#define ST_TOUCH_SET_NV                 10      ///< Write to Non Volatile Memory
#define ST_TOUCH_GET_NV                 11      ///< Read from Non Volatile Memory
#define ST_TOUCH_BULK_ERASE             21      ///< Erase entire NVM
#define ST_TOUCH_GET_NV_HUGE            22      ///< Read huge area of NVM
#define ST_TOUCH_GET_FLASH_ID           26      ///< Read ID from external flash
#define ST_TOUCH_SET_ENCRYPTION         27      ///< Turn encryption on
/** @} */

#define MAX_NUM_MINUTIAE                70      ///< Maximum minutia to be used for anti-latency
#define MAX_BAD_LINES                   28      ///< Maximum bad rows and bad columns
#define MAXBADPIXELS                    MAX_BAD_PIX ///< Maximum number of bad pixels
#define MAX_SETTINGS                    32      ///< Setting table size
#define DEF_CAL_SETTING                 15      ///< Default setting for calibration
#define DEF_STD_SETTING                 17      ///< Default setting for Standard reader mode
#define DEF_EIM_SETTING                 15      ///< Default setting for EIM/EIM-Lite reader modes

/**
 * @defgroup calibrationbitmask Calibration Bit Mask Values
 *
 * Values for bit mask in @ref TC_SensorTest
 * @{
 */
#define ST_BAD_LINES                    0x00000001  ///< Bad lines calibration
#define ST_BAD_PIXELS                   0x00000002  ///< Bad pixels calibration
#define ST_GAIN_OFFSET                  0x00000004  ///< Gain/offset calibration
#define ST_GRADIENT_PROFILE             0x00000008  ///< Gradient profile calibration
/** @} */

/**
 * @defgroup tcioverrides TCI TC_OverRideFlag Definitions
 * @{
 */
#define TC_FINGER_OVERRIDE              0x00000001  ///< Disable finger detection 
#define TC_GAIN_OVERRIDE                0x00000002  ///< Disable updating of the settings table
#define TC_NVM_USER_SPACE_OVERRIDE      0x00000004  ///< Allow access to full NVM
#define TC_REPAIRS_OVERRIDE             0x00000008  ///< Disable deploying repairs for Open(NONE)
#define TC_NVM_READ_ONLY                0x00000010  ///< Forces STSensorTest to just read the data from NVM
#define TC_FORCE_COATED                 0x00000020  ///< Obsolete...this space available
#define TC_OTF_OVERRIDE                 0x00000020  ///< Reused...override on-the-fly detection
#define TC_DIG_GAIN_OVERRIDE            0x00000040  ///< Skip the digital gain enhancement
#define TC_ORIENTATION_OVERRIDE         0x00000080  ///< Sensor orientation
#define TC_SC_DETECT_OVERRIDE           0x00000100  ///< Disable detection/repair of shorted columns
#define TC_USE_83_FINGER_DETECT         0x00000200  ///< Use the finger detection mechanism of PP8.3 (instead of the FP algo mechanism)
#define TC_EIMGRADIENT_OVERRIDE         0x00000400  ///< Skip EIM gradient compensation in EIM
#define TC_FINGER_TYPE_OVERRIDE         0x00000800  ///< Skip finger type detection algorithm
#define TC_DO_BEZEL_CONNECTION_TEST     0x00001000  ///< Perform bezel connection test
#define TC_EIMGRAD_SYSEXPORT_OVERRIDE   0x00002000  ///< Skip export of EIM gradient from NVM to system
/** @} */

/**
 * @defgroup tciconfigflag TCI ConfigFlag Definitions
 * @{
 */
#define TC_CFG_PIXEL_REPLACE            0x01    ///< Missing pixel replacement
#define TC_CFG_COATED_SENSOR            0x02    ///< Coated sensor
/** @} */

/**
 * @addtogroup oemfieldflags OEM Field Flags
 *
 * Internal OEM Field flags
 * @{
 */
#define OEM_FLAG_STFIELD                0x04
/** @} */

/**
 * @defgroup shortcolumn Shorted Column Detection Parameters
 *
 * Useful define for the shorted column detection
 *  - MSB is the threshold (MSB * 16 == threshold)
 *  - LSB indicates the OFFSET value to use during detection ((LSB * 4) + 3 == OFFSET)
 * @{
 */
#define SCDET_TH128_OFF63               0x8F    ///< Threshold 128, Offset 63...Standard values to use
#define SCDET_TH144_OFF63               0x9F    ///< Threshold 144, Offset 63
#define SCDET_TH128_OFF35               0x88    ///< Threshold 128, Offset 35
// ...and so on.
/** @} */

/**
 * @defgroup readertypes Reader Types
 * @{
 */
#define READER_TYPE_UNKNOWN             0       ///< Unknown reader
#define READER_TYPE_ST9                 1       ///< ST9 reader
#define READER_TYPE_BLACKFOREST         2       ///< Blackforest reader
#define READER_TYPE_CYPRESS             3       ///< Cypress reader
#define READER_TYPE_STM32               4       ///< STM32 reader
/** @} */

/**
 * @defgroup sensortypes Sensor Types
 * @{
 */
#define SENSOR_TYPE_UNKNOWN             0       ///< Unknown sensor
#define SENSOR_TYPE_C                   6       ///< Standard C sensor
#define SENSOR_TYPE_C_STEELCOAT         7       ///< Steelcoat C sensor
/** @} */

/**
 * @defgroup readermodeconstants Reader Mode Constants
 * @{
 */
#define READER_MODE_STD                 0       ///< Standard readers
#define READER_MODE_EIM                 1       ///< EIM readers utilizing full gradient
#define READER_MODE_EIM_LITE            2       ///< EIM readers utilizing simplified (average) gradient
/** @} */

/**
 * @defgroup readerprofileconstants Reader Profile Constants
 * @{
 */
#define NVM_EIMGRADIENT_LOC_CYPRESS     (32*1024)   ///< Location in NVM to store EIM gradient for Cypress
#define NVM_EIMGRADIENT_LOC_STM32       (34*1024)   ///< Location in NVM to store EIM gradient for STM32
/** @} */

/**
 * @defgroup eimgradvals EIM Gradient Values
 *
 * These values are used to create Windows registry keys and external files 
 * for saving and loading the full EIM gradient.
 * @{
 */
#define PERFECT_PRINT_SUBKEY            "Software\\UPEK\\PerfectPrint\\EIM"     ///< Subdirectory in Windows Registry for PerfectPrint
#if defined(VERSION_LINUX) || defined(VERSION_UNIX)
#define PERFECT_PRINT_SUBDIR            "/usr/local/bin/upek/eim/"              ///< Subdirectory in Linux file structure for PerfectPrint
#endif
#define STANDARD_EIM_GRADIENT_FILE_NAME "eimgrad"                               ///< Standard EIM gradient file name
/** @} */


//-------------------------------- Structures -------------------------------
/// Data structure for window settings
typedef struct
{
    unsigned int MaxWidth;          ///< Maximum width for the current sensor
    unsigned int MaxHeight;         ///< Maximum height for the current sensor
    unsigned int StartX;
    unsigned int StopX;
    unsigned int StartY;
    unsigned int StopY;
    unsigned int Height;
    unsigned int Width;
    unsigned int DeltaX;
    unsigned int DeltaY;
} WindowSettings;

/// Values for @ref ReadLogicalRegister and @ref WriteLogicalRegister
typedef enum
{
    NoForce = 0,                    ///< Read/write the intermal copy of the register
    Force,                          ///< Read/write from the actual registers
    ForceSingleReg                  ///< Read/write just the single actual register
} ReadWriteOpt;

/// Settings for calibration
enum CalSetType
{
    White = 0,
    Grey,
    Black,
    MAX_CAL_SETT
};

/// Status definitions
enum TCI_StatusDef
{
    NotInUse=0,
    PP_Initialize,
    PP_Open,
    PP_Authentify,
    PP_Close,
    PP_Terminate
};
#define PP_StatusDef BYTE

/// Gradient profile
typedef struct _GradientProfile
{
    BYTE first_init;                ///< Flag for first initialization
    BYTE num_blocks;                ///< Number of chunks in profile
    BYTE i_val[11];                 ///< Interpolation point values
    BYTE loc_offset[11];            ///< Offset to the next interpolation point.
    int min_pos;                    ///< Location of gradient minimum
    int GP[YMAX];                   ///< Reconstructed gradient profile
    int GP_Max;                     ///< Maximum gradient point in profile
} GradientProfile, FAR *LPGradientProfile, *PGradientProfile;

/// Reader profile 
typedef struct _ReaderProfile
{
    BYTE Version;                   ///< Structure version
    WORD PPRevision;                ///< Perfect Print version
    BYTE ReaderType;                ///< Reader type - @ref readertypes
    BYTE SensorType;                ///< Sensor type - @ref sensortypes
    BYTE ReaderMode;                ///< Reader mode (0=STD, 1=EIM, 2=EIM-Lite) - @ref readermodeconstants
    BYTE EIMOffset;                 ///< EIM offset (0=EIM not calibrated)
    BYTE EIMBaseOffset;             ///< Offset used to calculate EIM offset
    BYTE EIMGradCheckSum;           ///< Checksum for full EIM gradient
    BYTE EIMGradRowAverage[YMAX];   ///< Row average EIM gradient value for each sensor row
    DWORD EIMGradLoc;               ///< EIM gradient location in NVM
    BYTE EIMGrad[XMAX*YMAX];        ///< EIM gradient
} ReaderProfile, FAR *LPReaderProfile, *PReaderProfile;

// NVM structures

/// Reader profile structure in NVM
typedef struct _NVM_READER_PROFILE
{
    WORD Length;                    ///< Length of structure
    BYTE CheckSum;                  ///< Structure checksum
    BYTE Version;                   ///< Structure version
    WORD PPRevision;                ///< Perfect Print version
    BYTE ReaderType;                ///< Reader type - @ref readertypes
    BYTE SensorType;                ///< Sensor type - @ref sensortypes
    BYTE ReaderMode;                ///< Reader mode (0=STD, 1=EIM, 2=EIM-Lite) - @ref readermodeconstants
    BYTE EIMOffset;                 ///< EIM offset (0=EIM not calibrated)
    BYTE EIMBaseOffset;             ///< Offset used to calculate EIM offset
    BYTE EIMGradCheckSum;           ///< Checksum for full EIM gradient
    BYTE EIMGradRowAverage[YMAX];   ///< Row average EIM gradient value for each sensor row
    DWORD EIMGradLoc;               ///< EIM gradient location in NVM
    BYTE ReservedBuff[16];
} NVM_READER_PROFILE, FAR *LPNVM_READER_PROFILER, *PNVM_READER_PROFILE;

typedef struct _NVM_REC_PTR_FIELD
{
    BYTE RecType;                   ///< Defines type of record pointed to
    BYTE RecLoc;                    ///< Defines the starting address of the record (from 0) in 4-byte units.
} NVM_REC_PTR_FIELD, FAR *LPNVM_REC_PTR_FIELDR, *PNVM_REC_PTR_FIELD;

typedef struct _NVM_HEADER
{
    BYTE RecPtrOffset;              ///< Offset to first Record Pointer Field
    BYTE NumRecs;                   ///< Number of Record Pointer Fields
    BYTE UserStart;                 ///< Defines start of user space in 64-byte units.
    BYTE NVMSize;                   ///< Defines size of NVM in 512-byte units.
    BYTE DeviceType;                ///< Codes functionality of the device
    BYTE ValidByte;                 ///< Equal to 0xEE when EEPROM format is valid
    BYTE RevHVer;                   ///< Revision H sensor version information.
    BYTE Reserved;
} NVM_HEADER, FAR *LPNVM_HEADER, *PNVM_HEADER;

// size of this is 58 bytes....max size of one read packet.
#define MAX_NVMRECS  25
typedef struct _NVM_DIR
{
    NVM_HEADER          nvmh;
    NVM_REC_PTR_FIELD   rfp[MAX_NVMRECS];
} NVM_DIR, FAR *LPNVM_DIR, *PNVM_DIR;

/// List of record types in NVM
typedef enum
{
    RepairInfo = 0x10,
    SerialNumber,
    PixRepInfo,
    SerNumBackup,
    GainOffsetInfo,
    GradProfInfo,
    LatentInfo,
    OEMInfo,
    ReaderProfInfo
} NVMRecordType;

/// Structure for public/private data
typedef struct
{
    /// Public data
    struct
    {
        // public data
        BYTE SensorVersion;         ///< H1/TCS2A - @ref sensorversions
        BYTE DeviceType;            ///< USB/PP - @ref devicetypes
        BYTE USB_BusAddress;
        WindowSettings *Win;
    } Pub;
    /// Private data
    struct
    {
        // private data
        BYTE reader_num;            ///< Active reader entry
        BYTE first_init;            ///< Flag for first initilization
        BYTE ChipState;             ///< Imaging/Sleep - @ref chipstates
        BYTE FirmwareVersion;       ///< Reader firmware - @ref fwversions
        BYTE PeriodWidth;           ///< TC readout timing
        BYTE ChargeWidth;           ///< TC readout timing
        BYTE ResetWidth;            ///< TC readout timing
        BYTE ConfigFlag;            ///< Overrides read in from NVM - @ref tciconfigflag
        BYTE Orientation;           ///< Sensor orientation - @ref orientpositions
        BYTE IDD_Init;              ///< TC Current limit without any bad lines repaired
        BYTE IDD_Dynamic;           ///< TC Current limit after repairs
        BYTE D_gain[256];           ///< Digital gain lookup table
        BYTE FFD_gain[256];         ///< Faint finger digital gain lookup table
        PP_StatusDef PP_IntStatus;  ///< Internal status of PerfectPrint: initialize/authentify/open... - @ref TCI_StatusDef
        int TotalSettings;          ///< Maximum number of settings = 32
        int CurrentSetting;         ///< ex TCSETT = current setting
        int TC_OverRideFlags;       ///< Bit mask for overriding flags - @ref tcioverrides
        NVM_DIR nvmdir;             ///< Cached copy of NVM directory
        int *BadRow;                ///< Bad rows
        int *BadCol;                ///< Bad columns
        BadPix *gBadPixelList;      ///< Bad pixels
        GainOffsetStruct *GainOffset;   ///< Gain/offset calibration
        GradientProfile *gp;        ///< Gradient profile
        ReaderProfile *rp;          ///< Reader profile
        setting *TCSettingTable;    ///< Setting table
        setting *CalSettings;       ///< Calibration settings for the current sensor
        BOOL isCommunicationError;  ///< To stop working with the sensor when an error occurrs
        HANDLE hFingerEvent;
#if !defined(VERSION_LINUX) && !defined(VERSION_UNIX)
        CRITICAL_SECTION g_CriticalSection; ///< Handle to a critical section
#endif
        DeviceInfoStruct dev_info;  ///< Device list info
        BOOL inEIMOTF;              ///< Process currently in OTF for EIM?
        unsigned int StaticFaintFingerThreshold;            ///< Fingers with contrast below this value are considered "faint/weak" for normal sensors (static digital gain)
        unsigned int DynamicNoFingerThreshold;              ///< Fingers with contrast below this value are considered as non-existent for normal sensors (dynamic digital gain)
        unsigned int DynamicFaintFingerThreshold;           ///< Fingers with contrast below this value are considered "faint/weak" for normal sensors (dynamic digital gain)
        unsigned int DynamicFaintFingerTargetContrast;      ///< Target contrast for faint finger image grabs after digital gain is applied for normal sensors (dynamic digital gain)
        unsigned int FingerTriggerThreshold;                ///< The tolerated difference amount between a background pixel and finger pixel for normal sensors
        unsigned int FingerBoxMinThreshold;                 ///< Image contrast is computed inside a fingerprint-contained box of this mimimum size for normal sensors
        unsigned int StaticSCFaintFingerThreshold;          ///< Fingers with contrast below this value are considered "faint/weak" for steelcoat sensors (static digital gain)
        unsigned int DynamicSCNoFingerThreshold;            ///< Fingers with contrast below this value are considered as non-existent for steelcoat sensors (dynamic digital gain)
        unsigned int DynamicSCFaintFingerThreshold;         ///< Fingers with contrast below this value are considered "faint/weak" for steelcoat sensors (dynamic digital gain)
        unsigned int DynamicSCFaintFingerTargetContrast;    ///< Target contrast for faint finger image grabs after digital gain is applied for steelcoat sensors (dynamic digital gain)
        unsigned int SCFingerTriggerThreshold;              ///< The tolerated difference amount between a background pixel and finger pixel for steelcoat sensors
        unsigned int SCFingerBoxMinThreshold;               ///< Image contrast is computed inside a fingerprint-contained box of this mimimum size for steelcoat sensors
        BOOL         UseDynamicDigitalGainForFaintFinger;   ///< Use dynamic digital gain compensation rather than static with faint finger image grabs
        BOOL         ApplyDigitalGain;                      ///< Apply digital gain compensation to steelcoat sensors and faint finger image grabs
        BOOL         OnTheFlyRepairEnabled;                 ///< On-the-fly repair enabled during image grabbing
        BOOL         STM32_EncryptionEnabled;               ///< Encryption enabled for NVM access and image grabbing for STM32 devices
    } Prv;
} GlobalDef;

// Define the global variable
extern GlobalDef Glbv;

// Define shortcuts
#define GPub Glbv.Pub               ///< Public global variables
#define GlbWin Glbv.Pub.Win         ///< Global window settings
#define GPrv Glbv.Prv               ///< Private global variables


//--------------------------- Function prototypes ---------------------------

/* Assume C declarations for C++ callers */
#ifdef __cplusplus
extern "C" {
#endif


// Functions exported for internal use only

/**
 * @brief The function returns the value of the logical register 
 * (exported for internal use only).
 *
 * @param RegNum    Register number to be read (logical)
 * @param Data      Data read from the register
 *
 * @see ReadLogicalRegister
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return (other values) Errors from functions called internally
 */
#if DLL_EXPORT_FEATURE
#if defined(VERSION_INT)
DllExport
#endif
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall ReadI2CRegST(int RegNum, int *Data);

/**
 * @brief The function update the logical register with the data given
 * in input (exported for internal use only).
 *
 * @param RegNum    Register number to be written (logical)
 * @param Data      Data to be written
 *
 * @see WriteLogicalRegister
 *
 * @return @ref STERR_OK upon success
 * @return (other values) Errors from functions called internally
 */
#if DLL_EXPORT_FEATURE
#if defined(VERSION_INT)
DllExport
#endif
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall WriteI2CRegST(int RegNum, int Data);

/**
 *  @brief The function returns the value of the logical register.
 *
 *  The function reads the internal register table instead of the actual register 
 *  (see also @ref ReadPhysicalUSBRegister). The calling function can force reading the 
 *  actual TC register setting the parameter @a forceSensorRead = @c Force.
 *
 *  @param RegNum           Register number to be read (logical)
 *  @param Data             Data read from the register.  Must be pre-allocated in memory
 *  @param forceSensorRead  Force reading from the actual register - @ref ReadWriteOpt
 *
 *  @see ReadPhysicalUSBRegister
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 *  @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 *  @return @ref STERR_BAD_DEVICE if no device has been defined/open
 *  @return (other values) Errors from functions called internally
 */
#if DLL_EXPORT_FEATURE
#if defined(VERSION_INT)
DllExport
#endif
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall ReadLogicalRegister(int RegNum, int *Data, int forceSensorRead);

/**
 *  @brief The function update the logical register with the data given in input.
 *
 *  The function updates the internal register table instead of the actual register 
 *  (see @ref WritePhysicalUSBRegister). The calling function can force writing the 
 *  actual TC register setting the parameter @a forceSensorWrite = @c Force.
 *
 *  @param RegNum            Register number to be written (logical)
 *  @param Data              Data to be written
 *  @param forceSensorWrite  Force the writing to the actual register - @ref ReadWriteOpt
 *
 *  @see WritePhysicalUSBRegister
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 *  @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 *  @return @ref STERR_BAD_DEVICE if no device has been defined/open
 *  @return (other values) Errors from functions called internally
 */
#if DLL_EXPORT_FEATURE
#if defined(VERSION_INT)
DllExport
#endif
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall WriteLogicalRegister(int RegNum, int Data, int forceSensorWrite);

/**
 * @brief The function sets one or more flags, depending on the input mask.
 *
 * If the bit is 1 -> the flag is set to 1, otherwise it is not affected.
 *
 * @param mask Bit mask that defines which flags are set
 *
 * @see tcioverrides
 */
#if DLL_EXPORT_FEATURE
#if defined(VERSION_INT)
DllExport
#endif
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
void __stdcall SetTCIFlags(int mask);

/**
 * @brief The function resets one or more flags, depending on the input mask.
 *
 * If the bit is 1 -> the flag is set to 0, otherwise it not affected.
 *
 * @param mask Bit mask that defines which which flags are reset
 *
 * @see tcioverrides
 */
#if DLL_EXPORT_FEATURE
#if defined(VERSION_INT)
DllExport
#endif
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
void __stdcall ResetTCIFlags(int mask);

/**
 * @brief The function grabs a raw image from the sensor, without doing any 
 * further elaborations. 
 *
 * @param Buffer        Image read from the sensor
 * @param BufferSize    Size of the image to be grabbed
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_BAD_STATE if an 'ESD Event'/'USB Err. Handle' occurred
 * @return @ref STERR_BAD_DEVICE if no device has been defined/open
 * @return (other values) Error codes returned by the Grab function and other internal functions
 */
#if DLL_EXPORT_FEATURE
#if defined(VERSION_INT)
DllExport
#endif
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall GrabRawImage(unsigned char *Buffer, unsigned long BufferSize);

/**
 * @brief Retrieve the uncompressed PP settings table
 *
 * @param STab Settings table in which to store the values.  Must be
 *             pre-allocated in memory for @a Numsettings settings.
 * @param Numsettings Number of settings to get
 */
#if DLL_EXPORT_FEATURE
#if defined(VERSION_INT)
DllExport
#endif
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
void __stdcall GetSettingsTable(setting *STab, int Numsettings);

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
#if DLL_EXPORT_FEATURE
#if defined(VERSION_INT)
DllExport
#endif
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall GetReaderMode(int *mode);

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
#if DLL_EXPORT_FEATURE
#if defined(VERSION_INT)
DllExport
#endif
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall GetReaderType(int *type);

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
#if DLL_EXPORT_FEATURE
#if defined(VERSION_INT)
DllExport
#endif
#endif
#if APP_DLL_IMPORT_FEATURE
DllImport
#endif
int __stdcall GetSensorType(int *type);

// USB initialization

/**
 * @brief The function initializes the connection with the USB driver.
 *
 * @return @ref STERR_OK upon success
 * @return (other values) Errors from internal functions
 */
extern int InitializeUSB();

// USB declarations

#ifdef DLL_IMPORT_FEATURE
#include "usbdll.h"
#else
#include "usblib.h"
#endif

#ifdef __cplusplus
}
#endif

#endif  // #ifndef PERFECTPRINT_INT_H__

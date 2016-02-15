/**
 * @file ST_OEM.h
 *
 * @brief Structure of the UPEK OEM field in the NVM
 *
 * This is used for various PP data, including...
 * - compressed settings table
 * - TouchChip timing values
 * - Digital gain info
 * - sensor orientation flag
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef ST_OEM_H
#define ST_OEM_H


//-------------------------------- Structures -------------------------------
/**
 * @brief Hard-coded default values used in PerfectPrint
 *
 * The system data is a collection of default values used in PerfectPrint.  They
 * are stored in NVM at the time of reader calibration during manufacturing.  Any of 
 * these values can be overridden with the exception of @a PeriodWidth, @a ChargeWidth and 
 * @a ResetWidth for EIM readers.  The value 0 indicates that the default value 
 * should be used.
 */
typedef struct _SystemData
{
    CompressedSettingsTable STab;           ///< Used to fill in full settings table
    BYTE                    PeriodWidth;    ///< Timing value used for TC
    BYTE                    ChargeWidth;    ///< Timing value used for TC
    BYTE                    ResetWidth;     ///< Timing value used for TC
    BYTE                    Valid;          ///< Indicates that the field is Valid
    BYTE                    Orientation;    ///< Flags whether to interpolate missing pixels
    BYTE                    DigitalGain1;   ///< Used to construct lookup table for TCS1C gain compensation
    BYTE                    DigitalGain2;   ///< Used to construct lookup table for TCS1C gain compensation
    BYTE                    DigitalOffset;  ///< Used to construct lookup table for TCS1C gain compensation
    BYTE                    ConfigFlag;     ///< Can override pixel replacement, etc - @ref tciconfigflag
    BYTE                    ArrayWidth;     ///< Can override TCI value if not 0
    BYTE                    ArrayHeight;    ///< Can override TCI value if not 0
    BYTE                    SensorVersion;  ///< Can override TCI value if not 0 - @ref sensorversions
    BYTE                    AuthRetries;    ///< Authentify retries allowed. Can override ST9 default value if not 0
    BYTE                    IgnoreSTab;     ///< Ignore compressed settings table if not 0;
} SystemData, FAR *LPSystemData, *PSystemData;

/**
 * @brief Pads the system data structure size out to 64 bytes
 * @see _SystemData
 */
typedef struct _STOEMField
{
    union
    {
        SystemData sd;                      ///< The system data structure
        BYTE d[64];                         ///< This pads the structure size out to 64 bytes
    } sdb;
} STOEMField, FAR *LPSTOEMField, *PSTOEMField;

#endif  // #ifndef ST_OEM_H

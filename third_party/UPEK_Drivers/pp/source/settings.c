/**
 * @file settings.c
 * Functions to manipulate settings table for the TouchChip
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#include "perfectprintint.h"
#include "touchip1a.h"
#include "touchip1c.h"
#include "touchip2a.h"
#include "touchip2c.h"
#include "nvm.h"
#include "qualeng.h"

int RetrieveGainOffInfo(GainOffsetStruct *gain_offset);


//--------------------------- Local Functions -------------------------------

/**
 * @brief The function updates the setting table for the current sensor, using 
 *        the parameters previously calculated and stored in NVM, and then send 
 *        it to the firmware.
 *
 * (The updating of the table is executed only if the GainOverrideFlag is not 
 * set - @ref tcioverrides.)
 *
 * @param SettingTable (IN) setting table to be updated,
 *                     (OUT) updated setting table
 * @param NumSettings Total settings
 * @param SensorType Sensor type (H1/2A) - @ref sensorversions
 */
static void UpdateSettingTable(setting *SettingTable, int NumSettings, int SensorType)
{
    // update the table with the new values
    if((Glbv.Prv.TC_OverRideFlags&TC_GAIN_OVERRIDE) == 0)
    {
        // modify the table (if the values were found)
        if(RetrieveGainOffInfo(Glbv.Prv.GainOffset) == STERR_OK)
        {
            AdaptSettings(SettingTable, NumSettings, Glbv.Prv.GainOffset, SensorType);
        }
    }
}


//-------------------------- Function definitions ---------------------------

/**
 * @brief Retrieve the uncompressed PP settings table
 *
 * @param STab Settings table in which to store the values.  Must be 
 *             pre-allocated in memory for desired number of settings.
 * @param Numsettings Number of settings to retrieve
 */
void GetSettingsTable(setting *STab, int Numsettings)
{
    memcpy(STab, Glbv.Prv.TCSettingTable, Numsettings*sizeof(setting));
}

/**
 * @brief Initialize the settings table STab from the compressed 
 *        table CSTab
 *
 * Each byte of the compressed data has two nibbles of step info:
 * OOCC OOCC.  Each nibble has two bits of charge step and two bits 
 * of offset step.
 *
 * @param STab Settings table in which to store the values.  Must be 
 *             pre-allocated in memory for max number of settings.
 * @param CSTab Compressed settings table
 */

void ExpandSettingsTable( setting *STab, CompressedSettingsTable *CSTab)
{
    int i, j;
    int charge, offset, gain;
    int nibcnt;
    int sign;
    unsigned int step;  // step is always positive
    BYTE temp;
    setting STab83[MAX_SETTINGS];

    // load the initial values
    charge = CSTab->charge0;    // initial charge could be negative
    offset = CSTab->offset0;
    gain   = CSTab->gain0;

    // now work through the table of delta values to get 8.3 table
    i = 0;
    j = 0;
    while(i < MAX_SETTINGS)
    {
        // read the next byte
        temp = CSTab->dat[j];
        for(nibcnt=0; nibcnt<=1; nibcnt++)
        {
            sign = ((charge >= 0)?(0):(1));
            STab83[i].iSignChrg = sign;
            STab83[i].iChrg = abs(charge);
            STab83[i].iGain = gain;
            STab83[i].iOffset = offset;
            i++;    // point to next setting

            // calculate the deltas
            step = (temp&0x03); // mask off two bits
            charge += step;     // increment charge
            temp >>= 2;         // shift
            step = (temp&0x03); // mask off two bits
            offset += step;     // increment offset
            temp >>= 2;         // shift
        }
        j++;
    }

    // now convert PP8.3 STable to PP8.4
    for(i=0; i<=(MAX_SETTINGS/3); i++)
    {
        // setting 10--0;
        STab[(MAX_SETTINGS/3)-i].iSignChrg  = STab83[(3*i)].iSignChrg;
        STab[(MAX_SETTINGS/3)-i].iGain      = STab83[(3*i)].iGain;
        STab[(MAX_SETTINGS/3)-i].iChrg      = STab83[(3*i)].iChrg;
        STab[(MAX_SETTINGS/3)-i].iOffset    = (STab83[(3*i)].iOffset-2);

        // setting 10--20;
        STab[(MAX_SETTINGS/3)+i].iSignChrg  = STab83[(3*i)].iSignChrg;
        STab[(MAX_SETTINGS/3)+i].iGain      = STab83[(3*i)].iGain;
        STab[(MAX_SETTINGS/3)+i].iChrg      = STab83[(3*i)].iChrg;
        STab[(MAX_SETTINGS/3)+i].iOffset    = STab83[(3*i)].iOffset;

        // setting 31--21;
        STab[MAX_SETTINGS-1-i].iSignChrg    = STab83[(3*i)].iSignChrg;
        STab[MAX_SETTINGS-1-i].iGain        = STab83[(3*i)].iGain;
        STab[MAX_SETTINGS-1-i].iChrg        = STab83[(3*i)].iChrg;
        STab[MAX_SETTINGS-1-i].iOffset      = (STab83[(3*i)].iOffset+2);
    }
}

/**
 * @brief The function initializes the global setting table
 *        and calibration settings depending on sensor type
 *
 * @param SensorVersion Sensor version to use when initializing settings 
 *                      table - @ref sensorversions
 */
void InitSettingsTable(int SensorVersion)
{
    switch(SensorVersion)
    {
    case S1A_SENSOR:
        InitSettingsSensor1A();
        break;
    case S1C_SENSOR:
        InitSettingsSensor1C();
        break;       
    case S2A_SENSOR:
        InitSettingsSensor2A();
        break;
    case S2C_SENSOR:
        InitSettingsSensor2C();
        break;
    case FUTURE_SENSOR1:
        InitSettingsSensor1A();
        break;
    case FUTURE_SENSOR2:
        InitSettingsSensor1C();
        break;
    default:
        InitSettingsSensor1A();
    }

    // update with calibrated gain/offset if available
    UpdateSettingTable(Glbv.Prv.TCSettingTable, Glbv.Prv.TotalSettings, SensorVersion);
}

/**
 * @brief The function updates the setting table for the current sensor, using 
 *        the parameters previously calculated and stored in NVM, and then send 
 *        it to the firmware.
 *
 * (The updating of the table is executed only if the GainOverrideFlag is not 
 * set - @ref tcioverrides, or if the Initialize had already been completed.)
 *
 * @param SettingTable Setting table to be downloaded
 * @param NumSettings Total settings
 *
 * @return @ref STERR_OK upon success
 * @return (other values) returned by internal functions (NVM)
 */
int DownloadSettingTable(setting *SettingTable, int NumSettings)
{
    unsigned char new_setting_table[(MAX_SETTINGS*3)];
    int i;

    // load the table to be dawnloaded
    for(i=0; i<Glbv.Prv.TotalSettings; i++)
    {
        new_setting_table[(i*3)]    = ((SettingTable[i].iSignChrg*128)+SettingTable[i].iChrg);
        new_setting_table[(i*3)+1]  = SettingTable[i].iOffset;
        new_setting_table[(i*3)+2]  = SettingTable[i].iGain;
    }

    return OnSendSettingsUSB(new_setting_table, NumSettings);
}

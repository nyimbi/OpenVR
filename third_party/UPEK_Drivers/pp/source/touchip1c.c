/**
 * @file touchip1c.c
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
#include "touchip.h"
#include "touchip1c.h"


//------------------------------- Globals -----------------------------------
/**
 * @defgroup tcs1csensorinitsettings TCS1C Sensor Initial Settings
 * Full calibration Initial settings for H1 Sensor
 * @{
 */
const static setting Initial_white_1C = {0, 10, 7, 19};
const static setting Initial_grey_1C  = {0, 31, 7, 34};
const static setting Initial_black_1C = {0, 42, 7, 35};
/** @} */


//-------------------------- Function definitions ---------------------------

/**
 * @brief Initialize TCS1C sensor settings.
 */
void InitSettingsSensor1C()
{
    int i;
    setting CalSettings_1C[MAX_CAL_SETT];

    // define the default settings for grabbing images
    if((Glbv.Prv.rp->ReaderMode == READER_MODE_STD) && (Glbv.Prv.rp->ReaderType != READER_TYPE_STM32))
    {
        Glbv.Prv.CurrentSetting = DEF_STD_SETTING;
    }
    else
    {
        Glbv.Prv.CurrentSetting = DEF_EIM_SETTING;
    }

    // define the calibration settings for H1
    CalSettings_1C[White] = Initial_white_1C;
    CalSettings_1C[Grey]  = Initial_grey_1C;
    CalSettings_1C[Black] = Initial_black_1C;

    for(i=0; i<MAX_CAL_SETT; i++)
    {
        Glbv.Prv.CalSettings[i] = CalSettings_1C[i];
    }

    BuildDefaultTable();
}

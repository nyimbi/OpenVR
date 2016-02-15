/**
 * @file touchip1a.c
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
#include "touchip1a.h"


//------------------------------- Globals -----------------------------------
/**
 * @defgroup tcs1asensorinitsettings TCS1A Sensor Initial Settings
 * Full calibration Initial settings for H1 Sensor
 * @{
 */
const static setting Initial_white_1A = {0, 14, 7, 21};
const static setting Initial_grey_1A  = {0, 33, 7, 34};
const static setting Initial_black_1A = {0, 50, 7, 40};
/** @} */


//-------------------------- Function definitions ---------------------------

/**
 * @brief Initialize TCS1A sensor settings.
 */
void InitSettingsSensor1A()
{
    int i;
    setting CalSettings_1A[MAX_CAL_SETT];

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
    CalSettings_1A[White] = Initial_white_1A;
    CalSettings_1A[Grey]  = Initial_grey_1A;
    CalSettings_1A[Black] = Initial_black_1A;

    for(i=0; i<MAX_CAL_SETT; i++)
    {
        Glbv.Prv.CalSettings[i] = CalSettings_1A[i];
    }

    BuildDefaultTable();
}

/**
 * @file qualeng.c
 * @brief Quality engine functions
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */

#include "perfectprintint.h"
#include "ioreg.h"
#include "tci.h"
#include "nvm.h"
#include "badlines_new.h"
#include "touchip.h"
#include "readout_constants.h"
#include "qualeng.h"
#include "touchip1a.h"
#include "touchip1c.h"
#include "touchip2a.h"
#include "touchip2c.h"


//--------------------------- Local Functions -------------------------------
static void SettComp(unsigned long quality, int setting, unsigned long *best_quality, int *best_setting);

//-------------------------------- Constants --------------------------------

/**
 * @defgroup gradientprofileconsts Gradient Profile Constants
 * @{
 */
#define LEFT_SKIP       2
#define RIGHT_SKIP      2
#define ROW_CHUNK       36
/** @} */


//-------------------------- Function definitions ---------------------------

//---------------------------------------------------------------------------
/**
 * @brief Compares one quality with the best quality.
 *
 * If the tested quality is better than the best, it becomes the new best.
 *
 * @param quality       The quality to test against the best quality
 * @param setting       The setting that corresponds to the tested quality
 * @param best_quality  The best quality
 * @param best_setting  The best setting
 */
//---------------------------------------------------------------------------
void SettComp(unsigned long quality, int setting, unsigned long *best_quality, int *best_setting)
{
    if(quality > *best_quality)
    {
        *best_quality = quality;
        *best_setting = setting;
    }
}

//---------------------------------------------------------------------------
/**
 * @brief Gets the qualiy of the image
 *
 * @param ImageBuff Image from which to compute the quality
 * @param Width     Width of the image
 * @param Height    Height of the image
 *
 * @return The quality of the block
 */
//---------------------------------------------------------------------------
double GetQuality(unsigned char *ImageBuff, int Width, int Height)
{
    int z[9];
    int Gx, Gy, x, y, i, j, count, border=3;
    double A=0, B=0, C=0;
    double DataQualityA=0, dx, dy, dx2, dy2;

    // easy but slow way to get Gx and Gy
    for(x=(1+border); x<(Width-1-border); x++)
    {
        for(y=(1+border); y<(Height-1-border); y++)
        {
            // write on the z buffer to calculate local gradient
            count = 0;
            for(i=(x-1); i<(x+2); i++)
            {
                for(j=(y-1); j<(y+2); j++)
                {
                    z[count] = (*(ImageBuff+i+(j*Width)));
                    count++;
                }
            }

            // z[] pixel map:
            //       | 0 | 3 | 6 |
            //        ___ ___ ___
            //       | 1 | 4 | 7 |
            //        ___ ___ ___
            //       | 2 | 5 | 8 |
            Gx = ((z[6]+(2*z[7])+z[8])-(z[0]+(2*z[1])+z[2])) >> 2;
            Gy = ((z[2]+(2*z[5])+z[8])-(z[0]+(2*z[3])+z[6])) >> 2;

            // calculate the unnormalized sum signal as quality check for ridge
            dx = (double)Gx;
            dy = (double)Gy;
            dx2 = (dx*dx);
            dy2 = (dy*dy);
            A += dx2;
            B += dy2;
            C += (dx*dy);
            //DataQualityA += (((dx2-dy2)*(dx2-dy2)) + (4*dx2*dy2));
        }
    }

    DataQualityA = (A+B);

    return DataQualityA;
}

////////////////////////////////////////////////////////////////////////////
//                      Settings
////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
/**
 * @brief The function gets the best settings for the sensor
 *
 * For TCS1C sensor, it also does the digital gain compensation.
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_MEMORY_MALLOC_FAIL if memory allocation failed
 * @return (other values) Error codes returned by the Grab function and other internal functions
 */
//---------------------------------------------------------------------------
int GetBestSettings()
{
    int sett, best_setting=0;
    //int DeltaCharge=1;
    int iStatus=0;
    unsigned char *LocBuf;
    unsigned long QualityData[XMAX];
    unsigned long quality=0;
    unsigned long LocBufLen=(MOSAIC_WIDTH*MOSAIC_HEIGHT*Glbv.Prv.TotalSettings);

    LocBuf = (unsigned char *)malloc(sizeof(unsigned char)*LocBufLen);
    if(NULL == LocBuf)
    {
        return STERR_MEMORY_MALLOC_FAIL;
    }

    // grab the 32 small images
    iStatus = GrabMultipleImages(LocBuf, LocBufLen);
    if(iStatus!=STERR_OK)
    {
        free(LocBuf);
        return iStatus;
    }

    DigitalGainCompensate(LocBuf, LocBufLen);

    for(sett=0; sett < Glbv.Prv.TotalSettings; sett++)
    {
        QualityData[sett] = abs((int)(GetQuality((LocBuf+(MOSAIC_WIDTH*MOSAIC_HEIGHT)*sett), MOSAIC_WIDTH, MOSAIC_HEIGHT)));
        SettComp(QualityData[sett], sett, &quality, &best_setting);
    }

    free (LocBuf);

    iStatus = GetSelSettings(best_setting);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 * @brief The function sets the selected setting for the sensor
 *
 * @param SelSett The setting to select for the sensor
 *
 * @return @ref STERR_OK upon success
 * @return (other values) Error codes returned by other internal functions
 */
//---------------------------------------------------------------------------
int GetSelSettings(int SelSett)
{
    int iStatus;

    // select the SelSett settings in the TouchChip
    iStatus = WriteLogicalRegister(SETTINGS, SelSett, Force);
    if(iStatus != STERR_OK)
    {
        return iStatus;
    }

    // the setting in use now
    Glbv.Prv.CurrentSetting = SelSett;

    return STERR_OK;
}

////////////////////////////////////////////////////////////////////////////
//                      Gain/Offset calibration
////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
/**
 * @brief The function updates the charge and offset fields of the settings table
 * using the 4 double values passed in input.
 *
 * @param TCsett        Settings table
 * @param NumSettings   Total settings
 * @param gos           gain+offset read out / pixel
 * @param SensorType    Sensor type (TCS1A/TCS1C/TCS2A) - @ref sensorversions
 */
//---------------------------------------------------------------------------
void AdaptSettings(setting *TCsett, int NumSettings, GainOffsetStruct *gos, int SensorType)
{
    int i;
    double K_gain_ro, K_gain_pix, K_offset_ro, K_offset_pix;
    const int K_Min=0x0, K_Max=0x3F;

    switch(SensorType)
    {
    case S1A_SENSOR:
        K_gain_ro = S1A_GAIN_RO;
        K_gain_pix = S1A_GAIN_PIX;
        K_offset_ro = S1A_OFF_RO;
        K_offset_pix = S1A_OFF_PIX;
        break;
    case S1C_SENSOR:
        K_gain_ro = S1C_GAIN_RO;
        K_gain_pix = S1C_GAIN_PIX;
        K_offset_ro = S1C_OFF_RO;
        K_offset_pix = S1C_OFF_PIX;
        break;
    case S2A_SENSOR:
        K_gain_ro = S2A_GAIN_RO;
        K_gain_pix = S2A_GAIN_PIX;
        K_offset_ro = S2A_OFF_RO;
        K_offset_pix = S2A_OFF_PIX;
        break;
    case S2C_SENSOR:
        K_gain_ro = S2C_GAIN_RO;
        K_gain_pix = S2C_GAIN_PIX;
        K_offset_ro = S2C_OFF_RO;
        K_offset_pix = S2C_OFF_PIX;
        break;
    default:
        K_gain_ro = S1A_GAIN_RO;
        K_gain_pix = S1A_GAIN_PIX;
        K_offset_ro = S1A_OFF_RO;
        K_offset_pix = S1A_OFF_PIX;
        break;
    }

    for(i=0; i < NumSettings; i++)
    {
        TCsett[i].iChrg = (int)((TCsett[i].iChrg*K_gain_ro*K_gain_pix/(gos->gain_ro*gos->gain_pix)) - ((gos->offset_pix-K_offset_pix)/(gos->gain_pix*CHARGE_lsb)));
        if(TCsett[i].iChrg < K_Min)
        {
            TCsett[i].iChrg = K_Min;
        }
        else if(TCsett[i].iChrg > K_Max)
        {
            TCsett[i].iChrg = K_Max;
        }

        TCsett[i].iOffset = (int)(((TCsett[i].iOffset+K_offset_ro)*(1+K_gain_ro)/(1+gos->gain_ro)) - gos->offset_ro);
        if(TCsett[i].iOffset < K_Min)
        {
            TCsett[i].iOffset = K_Min;
        }
        else if(TCsett[i].iOffset > K_Max)
        {
            TCsett[i].iOffset = K_Max;
        }
    }
}

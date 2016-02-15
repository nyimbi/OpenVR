/**
 * @file orientation.c
 * Functions to do the orientation compensation
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
#include "orientation.h"


//-------------------------- Function definitions ---------------------------

//---------------------------------------------------------------------------
/**
 * @brief Convert Physical window to Logical window dependedent on orientation.
 *
 * @param p_LogWin  Pointer to the Logical window.  Must be pre-allocated in memory.
 * @param p_PhysWin Pointer to the Physical window
 *
 * @return @ref STERR_OK upon sucess
 * @return @ref STERR_BADPARAMETER if window data error
 */
//---------------------------------------------------------------------------
int MapPhysicalToLogicalWindow(WindowSettings *p_LogWin, WindowSettings *p_PhysWin)
{
    int ret=STERR_OK;

    if((Glbv.Prv.TC_OverRideFlags & TC_ORIENTATION_OVERRIDE) != 0)
    {
        memcpy(p_LogWin, p_PhysWin, sizeof(WindowSettings));
        return ret;
    }

    switch (Glbv.Prv.Orientation)
    {
        case ORIENT_ZERO:
            // logical and physical are the same (copy physical to logical)
            p_LogWin->DeltaX    = p_PhysWin->DeltaX;
            p_LogWin->DeltaY    = p_PhysWin->DeltaY;
            p_LogWin->Height    = p_PhysWin->Height;
            p_LogWin->Width     = p_PhysWin->Width;
            p_LogWin->MaxHeight = p_PhysWin->MaxHeight;
            p_LogWin->MaxWidth  = p_PhysWin->MaxWidth;

            p_LogWin->StartX    = p_PhysWin->StartX;
            p_LogWin->StartY    = p_PhysWin->StartY;
            p_LogWin->StopX     = p_PhysWin->StopX;
            p_LogWin->StopY     = p_PhysWin->StopY;
            break;

        case ORIENT_LEFT_90:
            // exchange x and y (reverse y)
            p_LogWin->DeltaX    = p_PhysWin->DeltaY;
            p_LogWin->DeltaY    = p_PhysWin->DeltaX;
            p_LogWin->Height    = p_PhysWin->Width;
            p_LogWin->Width     = p_PhysWin->Height;
            p_LogWin->MaxHeight = p_PhysWin->MaxWidth;
            p_LogWin->MaxWidth  = p_PhysWin->MaxHeight;

            p_LogWin->StartX    = p_PhysWin->StartY;
            p_LogWin->StartY    = (Glbv.Pub.Win->MaxWidth-1-p_PhysWin->StopX);
            p_LogWin->StopX     = p_PhysWin->StopY;
            p_LogWin->StopY     = (Glbv.Pub.Win->MaxWidth-1-p_PhysWin->StartX);
            break;

        case ORIENT_LEFT_180:
            // reverse x and y
            p_LogWin->DeltaX    = p_PhysWin->DeltaX;
            p_LogWin->DeltaY    = p_PhysWin->DeltaY;
            p_LogWin->Height    = p_PhysWin->Height;
            p_LogWin->Width     = p_PhysWin->Width;
            p_LogWin->MaxHeight = p_PhysWin->MaxHeight;
            p_LogWin->MaxWidth  = p_PhysWin->MaxWidth;

            p_LogWin->StartX    = (Glbv.Pub.Win->MaxWidth-1-p_PhysWin->StopX);
            p_LogWin->StartY    = (Glbv.Pub.Win->MaxHeight-1-p_PhysWin->StopY);
            p_LogWin->StopX     = (Glbv.Pub.Win->MaxWidth-1-p_PhysWin->StartX);
            p_LogWin->StopY     = (Glbv.Pub.Win->MaxHeight-1-p_PhysWin->StartY);
            break;

        case ORIENT_LEFT_270:
            // exchange x and y (reverse x)
            p_LogWin->DeltaX    = p_PhysWin->DeltaY;
            p_LogWin->DeltaY    = p_PhysWin->DeltaX;
            p_LogWin->Height    = p_PhysWin->Width;
            p_LogWin->Width     = p_PhysWin->Height;
            p_LogWin->MaxHeight = p_PhysWin->MaxWidth;
            p_LogWin->MaxWidth  = p_PhysWin->MaxHeight;

            p_LogWin->StartX    = Glbv.Pub.Win->MaxHeight-1-p_PhysWin->StopY;
            p_LogWin->StartY    = p_PhysWin->StartX;
            p_LogWin->StopX     = Glbv.Pub.Win->MaxHeight-1-p_PhysWin->StartY;
            p_LogWin->StopY     = p_PhysWin->StopX;
            break;

        default:
            ret = STERR_BADPARAMETER;
    }

    return ret;
}

//---------------------------------------------------------------------------
/**
 * @brief Convert Logical window to Physical window dependedent on orientation.
 *
 * @param p_PhysWin Pointer to the Physical window.  Must be pre-allocated in memory.
 * @param p_LogWin  Pointer to the Logical window
 *
 * @return @ref STERR_OK upon sucess
 * @return @ref STERR_BADPARAMETER if window data error
 */
//---------------------------------------------------------------------------
int MapLogicalToPhysicalWindow(PhyLogWinSetting *p_PhysWin, PhyLogWinSetting *p_LogWin)
{
    int ret=STERR_OK;

    if((Glbv.Prv.TC_OverRideFlags & TC_ORIENTATION_OVERRIDE) != 0)
    {
        memcpy(p_PhysWin, p_LogWin, sizeof(PhyLogWinSetting));
        return ret;
    }

    switch(Glbv.Prv.Orientation)
    {
        case ORIENT_ZERO:
            // logical and physical are the same (copy logical to physical)
            p_PhysWin->X0   = p_LogWin->X0;
            p_PhysWin->Y0   = p_LogWin->Y0;
            p_PhysWin->X    = p_LogWin->X;
            p_PhysWin->Y    = p_LogWin->Y;
            p_PhysWin->IncX = p_LogWin->IncX;
            p_PhysWin->IncY = p_LogWin->IncY;
            break;

        case ORIENT_LEFT_90:
            // exchange x and y (reverse y)
            p_PhysWin->X0   = (Glbv.Pub.Win->MaxWidth-(p_LogWin->Y0+p_LogWin->Y));
            p_PhysWin->Y0   = p_LogWin->X0;
            p_PhysWin->X    = p_LogWin->Y;
            p_PhysWin->Y    = p_LogWin->X;
            p_PhysWin->IncX = p_LogWin->IncY;
            p_PhysWin->IncY = p_LogWin->IncX;
            break;

        case ORIENT_LEFT_180:
            // reverse x and y
            p_PhysWin->X0   = (Glbv.Pub.Win->MaxWidth-(p_LogWin->X0+p_LogWin->X));
            p_PhysWin->Y0   = (Glbv.Pub.Win->MaxHeight-(p_LogWin->Y0+p_LogWin->Y));
            p_PhysWin->X    = p_LogWin->X;
            p_PhysWin->Y    = p_LogWin->Y;
            p_PhysWin->IncX = p_LogWin->IncX;
            p_PhysWin->IncY = p_LogWin->IncY;
            break;

        case ORIENT_LEFT_270:
            // exchange x and y (reverse x)
            p_PhysWin->X0   = p_LogWin->Y0;
            p_PhysWin->Y0   = (Glbv.Pub.Win->MaxHeight-(p_LogWin->X0+p_LogWin->X));
            p_PhysWin->X    = p_LogWin->Y;
            p_PhysWin->Y    = p_LogWin->X;
            p_PhysWin->IncX = p_LogWin->IncX;
            p_PhysWin->IncY = p_LogWin->IncY;
            break;

        default:
            ret = STERR_BADPARAMETER;
    }

    return ret;
}

//---------------------------------------------------------------------------
/**
 * @brief Convert Physical image to Logical image depended on orientation.
 *
 * @param logImg        Logical image.  Must be pre-allocated in memory to be 
 *                      of size >= @a BufferSize
 * @param physImg       Physical image
 * @param BufferSize    Image buffer size
 *
 * @return @ref STERR_OK upon sucess
 * @return @ref STERR_BADPARAMETER if window data error
 */
//---------------------------------------------------------------------------
int MapPhysicalToLogicalImage(unsigned char *logImg, unsigned char *physImg, int BufferSize)
{
    int i, j, ret=STERR_OK;

    if((Glbv.Prv.TC_OverRideFlags & TC_ORIENTATION_OVERRIDE) != 0)
    {
        memcpy(logImg, physImg, BufferSize);
        return ret;
    }

    switch (Glbv.Prv.Orientation)
    {
        case ORIENT_ZERO:
            // logical and physical are the same (copy logical to physical)
            for(i=0; i<BufferSize; i++)
            {
                j = i;
                logImg[j] = physImg[i];
            }
            break;

        case ORIENT_LEFT_90:
            // exchange x and y (reverse y)
            for(i=0; i<BufferSize; i++)
            {
                j = (((Glbv.Pub.Win->Width-1)-(i%Glbv.Pub.Win->Width))*Glbv.Pub.Win->Height+(i/Glbv.Pub.Win->Width));
                logImg[j] = physImg[i];
            }
            break;

        case ORIENT_LEFT_180:
            // reverse x and y
            for(i=0; i<BufferSize; i++)
            {
                j = (BufferSize-1-i);
                logImg[j] = physImg[i];
            }
            break;

        case ORIENT_LEFT_270:
            // exchange x and y (reverse x)
            for(i=0; i<BufferSize; i++)
            {
                j = ((i%Glbv.Pub.Win->Width)*Glbv.Pub.Win->Height+((Glbv.Pub.Win->Height-1)-(i/Glbv.Pub.Win->Width)));
                logImg[j] = physImg[i];
            }
            break;

        default:
            ret = STERR_BADPARAMETER;
    }

    return ret;
}

//---------------------------------------------------------------------------
/**
 * @brief Convert Logical image to Physical image dependedent on orientation.
 *
 * @param physImg       Physical image.  Must be pre-allocated in memory to 
 *                      be of size >= @a BufferSize
 * @param logImg        Logical image
 * @param BufferSize    Size of image to convert
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_BADPARAMETER if window data error
 */
//---------------------------------------------------------------------------
int MapLogicalToPhysicalImage(unsigned char *physImg, unsigned char *logImg, int BufferSize)
{
    int i, j, ret=STERR_OK;

    if((Glbv.Prv.TC_OverRideFlags & TC_ORIENTATION_OVERRIDE) != 0)
    {
        memcpy(physImg, logImg, BufferSize);
        return ret;
    }

    switch(Glbv.Prv.Orientation)
    {
        case ORIENT_ZERO:
            // logical and physical are the same (copy logical to physical)
            for(i=0; i<BufferSize; i++)
            {
                j = i;
                physImg[j] = logImg[i];
            }
            break;

        case ORIENT_LEFT_90:
            // exchange x and y (reverse y)
            for(i=0; i<BufferSize; i++)
            {
                j = (((Glbv.Pub.Win->Width-1)-(i%Glbv.Pub.Win->Width))*Glbv.Pub.Win->Height+(i/Glbv.Pub.Win->Width));
                physImg[i] = logImg[j];
            }
            break;

        case ORIENT_LEFT_180:
            // reverse x and y
            for(i=0; i<BufferSize; i++)
            {
                j = (BufferSize-1-i);
                physImg[i] = logImg[j];
            }
            break;

        case ORIENT_LEFT_270:
            // exchange x and y (reverse x)
            for (i=0; i<BufferSize; i++)
            {
                j = ((i%Glbv.Pub.Win->Width)*Glbv.Pub.Win->Height+((Glbv.Pub.Win->Height-1)-(i/Glbv.Pub.Win->Width)));
                physImg[i] = logImg[j];
            }
            break;

        default:
            ret = STERR_BADPARAMETER;
    }

    return ret;
}

//---------------------------------------------------------------------------
/**
 * @brief Convert Physical sensor data to Logical sensor data dependedent on orientation.
 *
 * @param p_LogSensorDat    Pointer to the Logical sensor data. Must be pre-allocated 
 *                          in memory
 * @param p_PhySensorDat    Pointer to the Physical sensor data
 *
 * @return @ref STERR_OK upon sucess
 * @return @ref STERR_BADPARAMETER if window data error
 */
//---------------------------------------------------------------------------
int MapPhysicalToLogicalSensorData(SensorDescr *p_LogSensorDat, SensorDescr *p_PhySensorDat)
{
    int ret=STERR_OK;

    if((Glbv.Prv.TC_OverRideFlags & TC_ORIENTATION_OVERRIDE) != 0)
    {
        memcpy(p_LogSensorDat, p_PhySensorDat, sizeof(SensorDescr));
        return ret;
    }

    p_LogSensorDat->SensorVersion = p_PhySensorDat->SensorVersion;

    switch(Glbv.Prv.Orientation)
    {
        case ORIENT_ZERO:
            // logical and physical are the same (copy logical to physical)
            p_LogSensorDat->ImageWidth  = p_PhySensorDat->ImageWidth;
            p_LogSensorDat->ImageHeight = p_PhySensorDat->ImageHeight;
            break;

        case ORIENT_LEFT_90:
            // exchange x and y (reverse y)
            p_LogSensorDat->ImageWidth  = p_PhySensorDat->ImageHeight;
            p_LogSensorDat->ImageHeight = p_PhySensorDat->ImageWidth;
            break;

        case ORIENT_LEFT_180:
            // reverse x and y
            p_LogSensorDat->ImageWidth  = p_PhySensorDat->ImageWidth;
            p_LogSensorDat->ImageHeight = p_PhySensorDat->ImageHeight;
            break;

        case ORIENT_LEFT_270:
            // exchange x and y (reverse x)
            p_LogSensorDat->ImageWidth  = p_PhySensorDat->ImageHeight;
            p_LogSensorDat->ImageHeight = p_PhySensorDat->ImageWidth;
            break;

        default:
            ret = STERR_BADPARAMETER;
    }

    return ret;
}

//---------------------------------------------------------------------------
/**
 * @brief Convert Physical repair data to Logical repair data dependedent on orientation.
 *
 * @param p_LogRepairDat    Pointer to the Logical repair data. Must be pre-allocated 
 *                          in memory
 * @param p_PhyRepairDat    Pointer to the Physical repair data
 *
 * @return @ref STERR_OK upon sucess
 * @return @ref STERR_BADPARAMETER if window data error
 */
//---------------------------------------------------------------------------
int MapPhysicalToLogicalRepairData(RepairDataStruct *p_LogRepairDat, RepairDataStruct *p_PhyRepairDat)
{
    int i, ret=STERR_OK;

    if((Glbv.Prv.TC_OverRideFlags & TC_ORIENTATION_OVERRIDE) != 0)
    {
        memcpy(p_LogRepairDat, p_PhyRepairDat, sizeof(RepairDataStruct));
        return ret;
    }

    if(p_PhyRepairDat->BadCol[0] > MAX_BAD_LINES)
    {
        return STERR_TOO_MANY_BAD_LINES;
    }
    if(p_PhyRepairDat->BadRow[0] > MAX_BAD_LINES)
    {
        return STERR_TOO_MANY_BAD_LINES;
    }

    // gain offset
    p_LogRepairDat->GainOffset.gain_pix     = p_PhyRepairDat->GainOffset.gain_pix;
    p_LogRepairDat->GainOffset.gain_ro      = p_PhyRepairDat->GainOffset.gain_ro;
    p_LogRepairDat->GainOffset.offset_pix   = p_PhyRepairDat->GainOffset.offset_pix;
    p_LogRepairDat->GainOffset.offset_ro    = p_PhyRepairDat->GainOffset.offset_ro;

    // gradient
    memset(p_LogRepairDat->Gradient, 0, YMAX);

    // bad pixels and bad rows/cols
    p_LogRepairDat->BadPixels.NumBadPixels  = p_PhyRepairDat->BadPixels.NumBadPixels;

    switch(Glbv.Prv.Orientation)
    {
        case ORIENT_ZERO:
            // logical and physical are the same (copy logical to physical)
            p_LogRepairDat->BadCol[0] = p_PhyRepairDat->BadCol[0];
            p_LogRepairDat->BadRow[0] = p_PhyRepairDat->BadRow[0];
            for(i=1; i<=p_LogRepairDat->BadCol[0]; i++)
            {
                p_LogRepairDat->BadCol[i] = p_PhyRepairDat->BadCol[i];
            }
            for(i=1; i<=p_LogRepairDat->BadRow[0]; i++)
            {
                p_LogRepairDat->BadRow[i] = p_PhyRepairDat->BadRow[i];
            }
            for(i=0; i<p_PhyRepairDat->BadPixels.NumBadPixels; i++)
            {
                p_LogRepairDat->BadPixels.BadPixLst[i].xcor = p_PhyRepairDat->BadPixels.BadPixLst[i].xcor;
                p_LogRepairDat->BadPixels.BadPixLst[i].ycor = p_PhyRepairDat->BadPixels.BadPixLst[i].ycor;
            }
            for(i=0; i<YMAX; i++)
            {
                p_LogRepairDat->Gradient[i] = p_PhyRepairDat->Gradient[i];
            }
            break;

        case ORIENT_LEFT_90:
            // exchange x and y (reverse y)
            p_LogRepairDat->BadCol[0] = p_PhyRepairDat->BadRow[0];
            p_LogRepairDat->BadRow[0] = p_PhyRepairDat->BadCol[0];
            for(i=1; i<=p_LogRepairDat->BadCol[0]; i++)
            {
                p_LogRepairDat->BadCol[i] = p_PhyRepairDat->BadRow[i];
            }
            for(i=1; i<=p_LogRepairDat->BadRow[0]; i++)
            {
                p_LogRepairDat->BadRow[i] = ((Glbv.Pub.Win->MaxWidth-1)-p_PhyRepairDat->BadCol[i]);
            }
            for(i=0; i<p_LogRepairDat->BadPixels.NumBadPixels; i++)
            {
                p_LogRepairDat->BadPixels.BadPixLst[i].xcor = p_PhyRepairDat->BadPixels.BadPixLst[i].ycor;
                p_LogRepairDat->BadPixels.BadPixLst[i].ycor = ((Glbv.Pub.Win->MaxWidth-1)-p_PhyRepairDat->BadPixels.BadPixLst[i].xcor);
            }
            for(i=0; i<YMAX; i++)
            {
                p_LogRepairDat->Gradient[i] = p_PhyRepairDat->Gradient[i];
            }
            break;

        case ORIENT_LEFT_180:
            // reverse x and y
            p_LogRepairDat->BadCol[0] = p_PhyRepairDat->BadCol[0];
            p_LogRepairDat->BadRow[0] = p_PhyRepairDat->BadRow[0];
            for(i=1; i<=p_LogRepairDat->BadCol[0]; i++)
            {
                p_LogRepairDat->BadCol[i] = ((Glbv.Pub.Win->MaxWidth-1)-p_PhyRepairDat->BadCol[i]);
            }
            for(i=1; i<=p_LogRepairDat->BadRow[0]; i++)
            {
                p_LogRepairDat->BadRow[i] = ((Glbv.Pub.Win->MaxHeight-1)-p_PhyRepairDat->BadRow[i]);
            }
            for(i=0; i<p_PhyRepairDat->BadPixels.NumBadPixels; i++)
            {
                p_LogRepairDat->BadPixels.BadPixLst[i].xcor = ((Glbv.Pub.Win->MaxWidth-1)-p_PhyRepairDat->BadPixels.BadPixLst[i].xcor);
                p_LogRepairDat->BadPixels.BadPixLst[i].ycor = ((Glbv.Pub.Win->MaxHeight-1)-p_PhyRepairDat->BadPixels.BadPixLst[i].ycor);
            }
            for(i=0; i<(int)Glbv.Pub.Win->MaxHeight; i++)
            {
                p_LogRepairDat->Gradient[Glbv.Pub.Win->MaxHeight-(i+1)] = p_PhyRepairDat->Gradient[i];
            }
            break;

        case ORIENT_LEFT_270:
            // exchange x and y (reverse x)
            p_LogRepairDat->BadCol[0] = p_PhyRepairDat->BadRow[0];
            p_LogRepairDat->BadRow[0] = p_PhyRepairDat->BadCol[0];
            for(i=1; i<=p_LogRepairDat->BadCol[0]; i++)
            {
                p_LogRepairDat->BadCol[i] = ((Glbv.Pub.Win->MaxHeight-1)-p_PhyRepairDat->BadRow[i]);
            }
            for(i=1; i<=p_LogRepairDat->BadRow[0]; i++)
            {
                p_LogRepairDat->BadRow[i] = p_PhyRepairDat->BadCol[i];
            }
            for(i=0; i<p_LogRepairDat->BadPixels.NumBadPixels; i++)
            {
                p_LogRepairDat->BadPixels.BadPixLst[i].xcor = ((Glbv.Pub.Win->MaxHeight-1)-p_PhyRepairDat->BadPixels.BadPixLst[i].ycor);
                p_LogRepairDat->BadPixels.BadPixLst[i].ycor = p_PhyRepairDat->BadPixels.BadPixLst[i].xcor;
            }
            for(i=0; i<(int)Glbv.Pub.Win->MaxHeight; i++)
            {
                p_LogRepairDat->Gradient[Glbv.Pub.Win->MaxHeight-(i+1)] = p_PhyRepairDat->Gradient[i];
            }
            break;

        default:
            ret = STERR_BADPARAMETER;
    }

    p_LogRepairDat->BadPixels.BadPixLst[MAXBADPIXELS-1].ycor = p_PhyRepairDat->BadPixels.BadPixLst[MAXBADPIXELS-1].ycor;

    return ret;
}

//---------------------------------------------------------------------------
/**
 * @brief Convert Physical EIM gradient data to Logical EIM gradient data 
 *        dependedent on orientation.
 *
 * @param p_LogEIMGradientDat       Logical EIM gradient data.  Must be 
 *                                  pre-allocated in memory
 * @param p_PhyEIMGradientDat       Physical EIM gradient data
 * @param p_LogEIMRowAverageDat     Logical EIM gradient row average data.  
 *                                  Must be pre-allocated in memory
 * @param p_PhyEIMRowAverageDat     Physical EIM gradient data
 *
 * @return @ref STERR_OK upon sucess
 * @return @ref STERR_BADPARAMETER if window data error
 */
//---------------------------------------------------------------------------
int MapPhysicalToLogicalEIMGradientData(BYTE *p_LogEIMGradientDat, BYTE *p_PhyEIMGradientDat, BYTE *p_LogEIMRowAverageDat, BYTE *p_PhyEIMRowAverageDat)
{
    int i, j, ret=STERR_OK;

    if((Glbv.Prv.TC_OverRideFlags & TC_ORIENTATION_OVERRIDE) != 0)
    {
        memcpy(p_LogEIMGradientDat, p_PhyEIMGradientDat, XMAX*YMAX);
        memcpy(p_LogEIMRowAverageDat, p_PhyEIMRowAverageDat, YMAX);
        return ret;
    }

    // gradients
    memset(p_LogEIMGradientDat, 0, XMAX*YMAX);
    memset(p_LogEIMRowAverageDat, 0, YMAX);

    switch (Glbv.Prv.Orientation)
    {
        case ORIENT_ZERO:
            // logical and physical are the same (copy logical to physical)
            for(i=0; i<XMAX*YMAX; i++)
            {
                p_LogEIMGradientDat[i] = p_PhyEIMGradientDat[i];
            }
            for(i=0; i<YMAX; i++)
            {
                p_LogEIMRowAverageDat[i] = p_PhyEIMRowAverageDat[i];
            }
            break;

        case ORIENT_LEFT_90:
            // exchange x and y (reverse y)
            for(i=0; i<XMAX*YMAX; i++)
            {
                j = (((Glbv.Pub.Win->MaxWidth-1)-(i%Glbv.Pub.Win->MaxWidth))*Glbv.Pub.Win->MaxHeight+(i/Glbv.Pub.Win->MaxWidth));
                p_LogEIMGradientDat[j] = p_PhyEIMGradientDat[i];
            }
            for(i=0; i<YMAX; i++)
            {
                p_LogEIMRowAverageDat[i] = p_PhyEIMRowAverageDat[i];
            }
            break;

        case ORIENT_LEFT_180:
            // reverse x and y
            for(i=0; i<XMAX*YMAX; i++)
            {
                j = ((XMAX*YMAX)-1-i);
                p_LogEIMGradientDat[j] = p_PhyEIMGradientDat[i];
            }
            for(i=0; i<YMAX; i++)
            {
                p_LogEIMRowAverageDat[YMAX-1-i] = p_PhyEIMRowAverageDat[i];
            }
            break;

        case ORIENT_LEFT_270:
            // exchange x and y (reverse x)
            for(i=0; i<XMAX*YMAX; i++)
            {
                j = ((i%Glbv.Pub.Win->MaxWidth)*Glbv.Pub.Win->MaxHeight+((Glbv.Pub.Win->MaxHeight-1)-(i/Glbv.Pub.Win->MaxWidth)));
                p_LogEIMGradientDat[j]=p_PhyEIMGradientDat[i];
            }
            for(i=0; i<YMAX; i++)
            {
                p_LogEIMRowAverageDat[YMAX-1-i] = p_PhyEIMRowAverageDat[i];
            }
            break;

        default:
            ret = STERR_BADPARAMETER;
    }

    return ret;
}

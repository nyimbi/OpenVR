/**
 * @file badlines_new.h
 * @brief Constants and functions used to repair image based on sensor data.
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
#include "tci.h"
#include "ioreg.h"
#include "nvm.h"
#include "qualeng.h"
#include "badlines_new.h"
#include "fingerdetect.h"
#include "touchip.h"
#include "orientation.h"

//--------------------------- Local Functions -------------------------------
static int RetrieveBadLinesRepairInfo(int *BadRow, int *BadCol);
static int RetrieveBadPixelsRepairInfo(BadPix *BadPixels);
static int DeployRepairInfo(int *BadCol, int *BadRow);
static int roundaverage(unsigned char *imgin, UINT x, UINT y, UINT aver);
static int AverageRows(UCHAR *img, int SrcRow1, int SrcRow2, int DstRow);


//-------------------------- Function definitions ---------------------------

//---------------------------------------------------------------------------
/**
 *  @brief Repairs the bad rows and bad columns that have been stored in NVM.
 *
 *  The function returns an error even if no repair records are found in NVM.
 *
 *  @return @ref STERR_OK upon success
 *  @return (other values) Errors from functions called internally
 */
//---------------------------------------------------------------------------
int SensorRepair()
{
    int iResult;

    // check if the flag to override repairs has been set
    if ((Glbv.Prv.TC_OverRideFlags & TC_REPAIRS_OVERRIDE)==0)
    {
        // read the bad lines repair information
        iResult = RetrieveBadLinesRepairInfo(Glbv.Prv.BadRow, Glbv.Prv.BadCol);
        if(iResult==STERR_OK)
        {
            iResult = DeployRepairInfo(Glbv.Prv.BadCol, Glbv.Prv.BadRow);
            if(iResult!=STERR_OK)
            {
                return iResult;
            }
        }

        // read the bad pixels repair info
        iResult = RetrieveBadPixelsRepairInfo(Glbv.Prv.gBadPixelList);
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief Retieves the bad lines repair info from NVM
 *
 *  @param BadRow Array to be loaded with the bad rows.  Must be pre-allocated 
 *                in memory to accomodate max number of rows.
 *  @param BadCol Array to be loaded with the bad columns.  Must be pre-allocated 
 *                in memory to accomodate max number of columns.
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_INVALID_VALUE if the record was found but the information is not valid
 *  @return @ref STERR_RECORD_NOT_FOUND if the record was not found
 *  @return (other values) Errors from functions called internally
 */
//----------------------------------------------------------------------------
int RetrieveBadLinesRepairInfo(int *BadRow, int *BadCol)
{
    int iResult;

    iResult = NVM_ReadLineRepairInfo(BadRow, BadCol);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief Retieves the bad pixels repair info from NVM
 *
 *  @param BadPixels Structure to be loaded with bad pixels
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_INVALID_VALUE if the record was found but the information is not valid
 *  @return @ref STERR_RECORD_NOT_FOUND if the record was not found
 *  @return (other values) Errors from functions called internally
 */
//----------------------------------------------------------------------------
int RetrieveBadPixelsRepairInfo(BadPix *BadPixels)
{
    int iResult;

    iResult = NVM_ReadPixRepairInfo(BadPixels);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief Returns the values of the 4 values that define the settings of the 
 *  TC (gain/offset) from NVM
 *
 *  @param gain_offset The gain/offset structure to receive the read out/pixel values
 *
 *  @return @ref STERR_OK upon success
 *  @return (other values) Errors from functions called internally
 */
//----------------------------------------------------------------------------
int RetrieveGainOffInfo(GainOffsetStruct *gain_offset)
{
    BYTE buffer[4];
    int iResult;

    // read from NVM
    iResult = NVM_ReadGainOffInfo(buffer);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    // convert from byte to double
    gain_offset->offset_ro = (double)(buffer[0]&0x7F)*0.008;

    // check the sign (negative if first bit is 1)
    if((buffer[0]&0x80) > 0)
    {
        gain_offset->offset_ro = gain_offset->offset_ro*(-1);
    }

    // convert from byte to double
    gain_offset->offset_pix = (double)(buffer[1]&0x7F)*0.008;

    // check the sign (negative if first bit is 1)
    if((buffer[1]&0x80) > 0)
    {
        gain_offset->offset_pix = gain_offset->offset_pix*(-1);
    }

    gain_offset->gain_pix = (double)(buffer[2]*0.004);    // sign always positive
    gain_offset->gain_ro = (double)(buffer[3]*0.012);     // sign always positive

    return iResult;
}

//---------------------------------------------------------------------------
/**
 *  @brief Load the reader profile with the data stored in NVM.  Also, load 
 *  reader profile with the data stored in NVM (Cypress), Windows registry (ST9) 
 *  or external file (ST9) if in EIM.
 *
 *  @param rprofile    Reader profile structure to receive the loaded values
 *  @param loadEIMGrad Should include EIM gradient when loading reader profile?
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_MEMORY_MALLOC_FAIL if memory allocation failure
 *  @return @ref STERR_WIN_REG_FAIL if there is a Windows registry failure
 *  @return @ref STERR_FILE_IN_OUT_FAIL if there is a file I/O failure
 *  @return @ref STERR_SENSOR_NOT_CALIBRATED if sensor is not calibrated
 *  @return @ref STERR_BAD_GRADIENT_DATA if gradient data in NVM, Windows 
 *  registry or external file is missing/corrupt
 *  @return (other values) Errors from functions called internally
 */
//----------------------------------------------------------------------------
int LoadReaderProfile(ReaderProfile *rprofile, BOOL loadEIMGrad)
{
    int iResult = STERR_OK;

    // read from NVM
    iResult = NVM_ReadReaderProfile(rprofile, loadEIMGrad);
    if(iResult!=STERR_OK)
    {
        goto Done;
    }

    iResult = STERR_OK;

Done:
    return iResult;
}

//---------------------------------------------------------------------------
/**
 *  @brief Deploys the repairs to the TouchChip.
 *
 *  The function works differently depending if the firmare is new or not: if 
 *  it's new -> the information is already recorded in the NVM, and the deploy 
 *  is done just writing a value into a firmware register.  If the firmware is 
 *  old -> the "CorrectHardware" will repair all the bad rows and bad columns 
 *  given in input.
 *
 *  @param BadCol Bad column array 
 *  @param BadRow Bad row array 
 *
 *  @return @ref STERR_OK upon success
 *  @return (other values) Errors from functions called internally
 */
//----------------------------------------------------------------------------
int DeployRepairInfo(int *BadCol, int *BadRow)
{
    int iResult;

    // writing to the NV_REPAIR register forces the firmware to deploy 
    // the repairs to the TouchChip
    iResult = WriteLogicalRegister(NV_REPAIR, 1, Force);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function repairs any pixel in the image which is included in the 
 *  table of known bad pixels by averaging nearby pixels.
 *
 *  @param table Pointer to a table of previously detected bad pixels
 *  @param imgin Pointer to input image
 *
 *  @return @ref STERR_OK upon success
 */
//----------------------------------------------------------------------------
int RepairPixels(BadPix *table,unsigned char *imgin)
{
    UINT i, lastpixel, ulBufSize;
    USHORT x, y, xc, yc;
    unsigned char RepairedPixels[MAXBADPIXELS];
    UINT aver = 0;

    //ulBufSize = ((GlbWin->Height/GlbWin->DeltaY)*(GlbWin->Width/GlbWin->DeltaX));
    ulBufSize = (GlbWin->Height*GlbWin->Width);

    // first compute the average
    if(ulBufSize > (32*32))
    {
        // for a big buffer, use the central data
        for(i=((ulBufSize/2)-(16*32)); i<((ulBufSize/2)+(16*32)); i++)
        {
            aver += *(imgin+i);
        }
        aver = (aver/(32*32));
    }
    else
    {
        // for a small buffer, use ALL the data
        for(i=0; i<ulBufSize; i++)
        {
            aver += *(imgin+i);
        }
        aver = (aver/ulBufSize);
    }

    for(i=0; i<table->NumBadPixels; i++)
    {
        xc = table->BadPixLst[i].xcor;
        yc = table->BadPixLst[i].ycor;

        if((xc==0) && (yc==0))
        {
            break;
        }

        // normalize coordinates
        x = ((xc-GlbWin->StartX)/GlbWin->DeltaX);
        y = ((yc-GlbWin->StartY)/GlbWin->DeltaY);

        if((((xc-GlbWin->StartX)%GlbWin->DeltaX)==0) && (((yc-GlbWin->StartY)%GlbWin->DeltaY)==0))
        {
            if((x>0) && (x<(int)GlbWin->Width) && (y>0) && (y<(int)GlbWin->Height))
            {
                RepairedPixels[i] = roundaverage(imgin, x, y, aver);
            }
        }
    }

    // now copy all the repaired pixel data into the input buffer.
    lastpixel = i;
    for(i=0; i<lastpixel; i++)
    {
        xc = table->BadPixLst[i].xcor;
        yc = table->BadPixLst[i].ycor;

        // normalize coordinates
        x = ((xc-GlbWin->StartX)/GlbWin->DeltaX);
        y = ((yc-GlbWin->StartY)/GlbWin->DeltaY);

        if((x>0) && (x<(int)GlbWin->Width) && (y>0) && (y<(int)GlbWin->Height))
        {
            *(imgin+x+y*GlbWin->Width) = RepairedPixels[i];
        }
    }

    return STERR_OK;
}

//----------------------------------------------------------------------------
/**
 *  @brief Clears list of bad rows, bad columns and bad pixels
 */
//----------------------------------------------------------------------------
void InitializeRepairGlobals()
{
    memset(Glbv.Prv.BadRow, 0, YMAX*sizeof(Glbv.Prv.BadRow[0]));
    memset(Glbv.Prv.BadCol, 0, XMAX*sizeof(Glbv.Prv.BadCol[0]));
    memset(Glbv.Prv.gBadPixelList, 0, sizeof(Glbv.Prv.gBadPixelList));
}


//----------------------------------------------------------------------------
/**
 *  @brief Calculate average value to replace bad pixel value
 *
 *  @param imgin Pointer to input image
 *  @param x     Normalized x-coordinate of bad pixel
 *  @param y     Normalized y-coordinate of bad pixel
 *  @param aver  Average image pixel value for image
 *
 *  @return Average value for repaired pixel
 */
//----------------------------------------------------------------------------
int roundaverage(unsigned char *imgin, UINT x, UINT y, UINT aver)
{
    UINT p[4];
    int i, count, sum;

    p[0] = *(imgin+x-1+y*GlbWin->Width);
    p[1] = *(imgin+x+1+y*GlbWin->Width);
    p[2] = *(imgin+x+(y+1)*GlbWin->Width);
    p[3] = *(imgin+x+(y-1)*GlbWin->Width);

    count = 0;
    sum = 0;
    for(i=0; i<4; i++)
    {
        if((p[i]<(aver-(BADPIXELTH-10))) || (p[i]>(aver+(BADPIXELTH+10))))
        {
            // ignore this value...it is also bad
            ;
        }
        else
        {
            sum += p[i];
            count++;
        }
    }
    if(count == 0)
    {
        // don't divide by zero; if all surrounding pixels are bad, punt
        return aver;
    }

    // return average of nearby good pixels.
    return (sum/count);
}

//---------------------------------------------------------------------------
/**
 *  @brief The function repairs the rows following bad rows, doing the average
 *  of the previous and next row.
 *
 *	@todo The case that multiple bad rows are detected is not covered
 *
 *  @param BadRow     Array of bad rows 
 *  @param Buffer     Image grabbed from the sensor
 *  @param BufferSize Size of the image 
 *
 *  @return @ref STERR_OK upon success
 */
//--------------------------------------------------------------------------
int CorrectRowsAfterBadRows(int *BadRow, unsigned char *Buffer, unsigned long BufferSize)
{
    int i;
    UINT badr;

    for(i=1; i<=BadRow[0]; i++)
    {
        if((BadRow[i]>=(int)GlbWin->StartY) && (BadRow[i]<(int)GlbWin->StopY))
        {
            badr = (BadRow[i]+1-GlbWin->StartY);  //Physical to logical
            AverageRows(Buffer, badr-1, badr+1, badr);
        }
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function averages 2 rows together to replace the destination row
 *
 *  @param img     Image grabbed from the sensor and to receive the averaged row.
 *                 Must be pre-allocated in memory to accomodate window size.
 *  @param SrcRow1 The row number of the 1st row to average 
 *                 (addressing is considered absolute from 0 to 255)
 *  @param SrcRow2 The row number of the 2nd row to average 
 *                 (addressing is considered absolute from 0 to 255)
 *  @param DstRow  The destination row number to replace in img 
 *                 (addressing is considered absolute from 0 to 255)
 *
 *  @return @ref STERR_OK upon success
 */
//--------------------------------------------------------------------------
int AverageRows(UCHAR *img, int SrcRow1, int SrcRow2, int DstRow)
{
    UINT i;
    int temp;

    // addressing is considered absolute from 0 to 255
    if((DstRow<=(int)(GlbWin->StartY+GlbWin->Height-1)) && (DstRow>(int)GlbWin->StartY))
    {
        for(i=0; i<GlbWin->Width; i++)
        {
            temp = ((int)(*(img+SrcRow1*GlbWin->Width+i)) + (int)(*(img+SrcRow2*GlbWin->Width+i)));
            *(img+DstRow*GlbWin->Width+i) = (temp>>1);
        }
    }

    return STERR_OK;
}

//--------------------------------------------------------------------------
/**
 *  @brief Replaces the pixel of interest (POI) with the average of the 2 pixels 
 *  on the left and right of it.
 *
 *  Note that POI coordinates are global (sensor array) coordinates
 *
 *  @param ImgBuff    Image buffer for interpolating
 *  @param arrayX     Absolute array column address of pixel
 *  @param arrayY     Absolute array row address of pixel
 *  @param rows       Number of rows in window
 *  @param row_offset X-coordinate of window origin
 *  @param delta_row  Delta rows
 *  @param cols       Number of columns in window
 *  @param col_offset Y-coordinate of window origin
 *  @param delta_col  Delta columns
 */
//--------------------------------------------------------------------------
void PixelAverage(BYTE *ImgBuff, int arrayX, int arrayY, int rows, int row_offset, int delta_row, int cols, int col_offset, int delta_col)
{
    BYTE *pbtemp;
    int imagewidth;
    int average;
    int count;

    imagewidth = cols; // width of image in pixels, not array coords
    average = 0;
    count = 0;

    // first point to the Pixel Of Interest (POI)
    pbtemp = (ImgBuff
                + ((arrayX-col_offset)/delta_col)                   // local x address
                + (((arrayY-row_offset)/delta_row)*imagewidth));    // local y address 

    // if POI is not on left edge of image
    if(arrayX > col_offset)
    {
        average += *(pbtemp-1); // average in the pixel to the left of POI
        count++;
    }

    // if POI is not on right edge of image
    if(arrayX < (col_offset+(cols-1)*delta_col))
    {
        average += *(pbtemp+1); // average in the pixel to the right of POI
        count++;
    }

    // don't divide by zero on a 1X1 image!
    if(count != 0)
    {
        average /= count;
    }

    // replace POI with average;
    *pbtemp = (BYTE)average;
}

//--------------------------------------------------------------------------
/**
 * @brief Moves through image data and replaces "missing" pixels with average 
 * some surrounding pixels ("missing" pixels are replaced with ESD structures.)
 *
 * @param ImgBuff Image buffer for interpolating.  Must be pre-allocated in memory
 *                to accomodate window size.
 * @param pWS     Pointer to the global window setting
 */
//--------------------------------------------------------------------------
void InterpolateMissingPixels(BYTE *ImgBuff, WindowSettings *pWS)
{
    int globalx;   // absolute array col address of pixel
    int globaly;   // absolute array row address of pixel
    int rows;
    int row_offset;
    int delta_row;
    int cols;
    int col_offset;
    int delta_col;
    int lastCol;

    if(Glbv.Pub.Win->MaxWidth > 208)
    {
        lastCol = MISSING_PIXEL_XSTOP_TCS1;
    }
    else
    {
        lastCol = MISSING_PIXEL_XSTOP_TCS2;
    }

    rows = pWS->Height;
    row_offset = pWS->StartY;
    delta_row = pWS->DeltaY;
    cols = pWS->Width;
    col_offset = pWS->StartX;
    delta_col = pWS->DeltaX;

    // for each missing pixel located in the window
    for(globalx=MISSING_PIXEL_XSTART; (globalx<=(col_offset+(cols-1)*delta_col)) && (globalx<=lastCol); globalx+=MISSING_PIXEL_XSTEP)
    {
        if(globalx < col_offset)
        {
            continue;
        }
        if(((globalx-col_offset)%delta_col) != 0)
        {
            continue;
        }

        for(globaly=MISSING_PIXEL_YSTART; globaly<=(row_offset+(rows-1)*delta_row); globaly+=MISSING_PIXEL_YSTEP)
        {
            if(globaly < row_offset)
            {
                continue;
            }
            if(((globaly - row_offset)%delta_col) != 0)
            {
                continue;
            }
            //printf("testing x=%d, y=%d\n", globalx, globaly);

            // replace the data with average from neighbors
            PixelAverage(ImgBuff, globalx, globaly, rows, row_offset, delta_row, cols, col_offset, delta_col);
        }
    }
}

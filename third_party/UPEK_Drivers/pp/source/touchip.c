/**
 * @file touchip.c
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
#include "fingerdetect.h"
#include "qualeng.h"
#include "touchip.h"
#include "ioreg.h"
#include "nvm.h"
#include "tci.h"
#include "orientation.h"

int CorrectRowsAfterBadRows(int *BadRow, unsigned char *Buffer, unsigned long BufferSize);
int RepairPixels(BadPix *table,unsigned char *imgin);

//-------------------------- Local Functions ----------------------------
static BOOL CreateFingerWindow(unsigned char *image, int imgWidth, int imgHeight, int *StartRow, int *EndRow, int *StartCol, int *EndCol);
static void ComputeImageHistogram(unsigned char *image, int imgWidth, int imgHeight, int StartRow, int EndRow, int StartCol, int EndCol, double *average, double *sigma, int *peakPosition, int *peakValue, int *widthofHalfHeight, int *histogram);
static int GetHistogramContrastBetween(int *histogram, double percentTH);

//----------------------------------------------------------------
/**
 * @brief Sets up the lookup table based on the two input parameters.
 *                  
 * @param        gain1   Fixed-point 8-bit number  
 *                       BBB.BBBBB (range from 0 to 7.96875) 
 *                       gain from unity point down
 *
 * @param        gain2   Fixed-point 8-bit number  
 *                       gain from unity point up
 *
 * @param        offsetparm Is the unity point for input/output   
 * @param        pDigGain Lookup table array.  Must be at least 256 bytes.
 */
//----------------------------------------------------------------

void InitDigitalGainTable(BYTE gain1, BYTE gain2, BYTE offsetparm, BYTE *pDigGain){
    int i;
    int temp;
    
    for (i=0;i<256;i++){
        temp = offsetparm - i;
        if (temp > 0)       // i < offsetparm
            temp*= -gain1;
        else
            temp*= -gain2;   // i >= offsetparm
        temp /= 32;         // fixed point normalization
        temp += offsetparm;
        temp = (temp > 254) ? 254 : temp;
        temp = (temp < 1) ? 1 : temp;
        pDigGain[i] = (BYTE) temp;
    }
    
}

//------------------------ Imaging Functions --------------------------------

//---------------------------------------------------------------------------
/**
 * @brief The function grabs an image from the sensor, and returns it to the calling
 * application only if a finger has been detected and is of good quality.
 *
 * Otherwise, returns the errors @c STERR_NO_FINGER and @c STERR_BAD_QUALITY_FINGER 
 * and the buffer may be cleared. For TCS1C sensor, it also interpolates the missing 
 * pixels and does the digital gain compensation.	
 *
 * @param	Buffer		Image read from the sensor
 * @param	BufferSize  Size of the image to be grabbed
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_BAD_DEVICE if no device has been defined/open
 * @return @ref STERR_NO_FINGER if no finger was detected on the sensor
 * @return @ref STERR_BAD_QUALITY_FINGER if the fingerprint returned was a bad quality image
 * @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 * @return (other values) Error codes returned by the Grab function and other internal functions
 */
//----------------------------------------------------------------------------
int GrabImage(unsigned char *Buffer, unsigned long BufferSize)
{
    int iStatus = STERR_OK;
    BOOL isFingerPresent;
    BOOL isBadQualityFinger;

    // default initialization when finger detection is not called
    isFingerPresent = TRUE;
    isBadQualityFinger = FALSE;

    // grab raw image
    {
        iStatus = GrabRawImage(Buffer, BufferSize);
    }

    if(iStatus!=STERR_OK)
    {
        goto Done;
    }

    // interpolate missing pixels
    if(Glbv.Prv.ConfigFlag & TC_CFG_PIXEL_REPLACE)
    {
        InterpolateMissingPixels(Buffer, Glbv.Pub.Win);
    }

    // repair bad rows and bad pixel
    if((Glbv.Prv.TC_OverRideFlags&TC_REPAIRS_OVERRIDE) == 0)
    {
        // correct bad rows following real bad rows
        CorrectRowsAfterBadRows(Glbv.Prv.BadRow, Buffer, BufferSize);

        // do pixel repair
        RepairPixels(Glbv.Prv.gBadPixelList, Buffer);
    }

    if((Glbv.Prv.TC_OverRideFlags&TC_FINGER_OVERRIDE) == 0)
    {
        // detect if finger is present or not (if override flag has not been set)
        iStatus = FingerDetect(Buffer,  GlbWin->Width,  GlbWin->Height, ((GlbWin->DeltaX)?(508/GlbWin->DeltaX):(508)), ((GlbWin->DeltaY)?(508/GlbWin->DeltaY):(508)), &isFingerPresent, &isBadQualityFinger);
        if((iStatus!=STERR_OK) && (iStatus!=STERR_NO_FINGER))
        {
            goto Done;
        }
    }

    // apply digital gain for standard mode
    if(isFingerPresent && !isBadQualityFinger && (Glbv.Prv.rp->ReaderMode == READER_MODE_STD) && (Glbv.Prv.rp->ReaderType != READER_TYPE_STM32))
    {
        DigitalGainCompensate(Buffer, BufferSize);
    }

    // return the status according to the finger detection
    if(isFingerPresent)
    {
        if (isBadQualityFinger)
        {
            iStatus = STERR_BAD_QUALITY_FINGER; // low quality finger
            memset(Buffer, 0x50, BufferSize);   // clear buffer
        }
        else
        {
            iStatus = STERR_OK;                 // good finger
        }
    }
    else
    {
        iStatus = STERR_NO_FINGER;              // no finger
        memset(Buffer, 0x50, BufferSize);       // clear buffer
    }

Done:
    return iStatus;
}

//---------------------------------------------------------------------------
/**
 * @brief The function grabs a raw image from the sensor, without doing any 
 *		  further elaborations. 
 *
 * @param Buffer		  Image read from the sensor
 * @param BufferSize	  Size of the image to be grabbed
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return @ref STERR_BAD_STATE if an 'ESD Event'/'USB Err. Handle' occurred
 * @return @ref STERR_BAD_DEVICE	if no device has been defined/open
 * @return (other values) Error codes returned by the Grab function and other internal functions
 */
//----------------------------------------------------------------------------
int GrabRawImage(unsigned char *Buffer, unsigned long BufferSize)
{
    int iResult;
    
    // Check Chip status (Check if RESET/"light" ESD event happened)
    iResult = Check_ESD_CommunicationError();
    if(iResult!=STERR_OK)
        return iResult;

    // check for NULL parameters
    if(Buffer == NULL)
        return STERR_BADPARAMETER;

    // grab the image
    switch(Glbv.Pub.DeviceType)
    {			
    case ST_NO_DEVICE:
        iResult=STERR_NODEVICE;
        break;
        
    case ST_USB:
        if (GPrv.FirmwareVersion < VERSION_24){
            iResult = OnGrabUSB(Buffer, BufferSize,0);
        }
        else { 
            iResult = OnGrabUSB(Buffer, BufferSize,GRAB_TYPE_NEW_GRAB);
        }
        Check_USB_CommunicationError(&iResult);
        break;
        
    default:
        iResult=STERR_BAD_DEVICE;			
        break;
    }
    
    if (iResult != STERR_OK) {
        // check for USB error
        return (iResult);
    }
    
    return iResult;
}


//---------------------------------------------------------------------------
/**
 * @brief The function grabs 32 small images (32x32) from the sensor, each one having
 * a different setting and positioned in the middle of the window. 
 *
 * The images are grabbed using the internal functionality of the @ref OnGrabUSB 
 * The returned @a Buffer will contain the list of the 32 small images.
 *
 * @param Buffer		  Image read from the sensor
 * @param BufferSize	Size of the image to be grabbed
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_BUFFER_TOO_SMALL if @a Buffer is not large enough
 * @return (other values) Error codes returned by the @ref OnGrabUSB function or other internal functions
 */
//----------------------------------------------------------------------------
int GrabMultipleImages(unsigned char *Buffer, unsigned long BufferSize)
{
    unsigned int LocBufSize;
    unsigned int X0,Y0;
    int iStatus=0;
    int currentsetting;
    WindowSettings winsett;
    
    LocBufSize = MOSAIC_WIDTH*MOSAIC_HEIGHT*Glbv.Prv.TotalSettings;
    
    if (BufferSize < LocBufSize)
        return STERR_BUFFER_TOO_SMALL;
    
    // check for ESD event
    iStatus = Check_ESD_CommunicationError();
    if(iStatus!=STERR_OK)
        return iStatus;
    
    SaveSetting(&currentsetting);
    SaveWindowSettings(&winsett);
    
    // set the window size and position
    X0=GlbWin->MaxWidth/2;
    Y0=GlbWin->MaxHeight/2;
    
    // the window setting is not automatically handled by 
    // the Firmware (set window 2 pixels wider that desired 
    // window and 2 pixels left of desired startCol )
    //	iStatus=SetWindow(128-2, 200, MOSAIC_WIDTH+2, MOSAIC_HEIGHT, 1, 1);
    iStatus=SetWindow(X0-18, Y0, MOSAIC_WIDTH+2, MOSAIC_HEIGHT, 1, 1);
    if (iStatus != STERR_OK)
        return iStatus;
    
    // Grab an image with 32 small images, each one of size 32x32
    
    if (GPrv.FirmwareVersion < VERSION_24){
        iStatus = OnGrabUSB(Buffer, LocBufSize,GRAB_TYPE_SETTINGS_GRAB);
    }        
    else { 
        iStatus = OnGrabUSB(Buffer, LocBufSize,GRAB_TYPE_NEW_GRAB+GRAB_TYPE_SETTINGS_GRAB);
    }
    
    if (iStatus!=STERR_OK) {
        RestoreSetting(&currentsetting);
        RestoreWindowSettings(&winsett);
        return iStatus;
    }
    
    // restore old window settings
    iStatus =RestoreSetting(&currentsetting);
    if (iStatus!=STERR_OK) return iStatus;
    iStatus =RestoreWindowSettings(&winsett);
    if (iStatus!=STERR_OK) return iStatus;
    
    return STERR_OK;
    
}

//------------------------ Windowing Functions ------------------------------

//---------------------------------------------------------------------------
/**
 * @brief The function sets the origin coordinates, size and sub sample size. 
 *
 * These values are then used by the next Grab function.
 *
 * @param X0		X coordinate of the window starting point
 * @param Y0		Y coordinate of the window starting point
 * @param X		  Width of the window
 * @param Y		  Height of the Window
 * @param IncX	Sub sample on the X axes
 * @param IncY	Sub sample on the Y axes
 *
 * @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 * @return (other values) Status from internal functions
 */
//----------------------------------------------------------------------------
int	SetWindow(int X0, int Y0, int X, int Y, int IncX, int IncY)
{
    // The functions set the window coordinate and dimension for the TouchChip
    int iStatus;
    int XSamples, YSamples;
	int locMaxWidth, locMaxHeight;
    
    // check for error settings
    
	//all of them have to be positive
	if (X0<0 || Y0<0 || X<=0 || Y<=0 || IncX<=0 || IncY<=0)
		return STERR_BADPARAMETER;

	locMaxWidth=GlbWin->MaxWidth;
	locMaxHeight=GlbWin->MaxHeight;

	//upper limit
	if (X0>=locMaxWidth || Y0>=locMaxHeight || X>locMaxWidth || Y>locMaxHeight)
		return STERR_BADPARAMETER;

    //over size 
    if((X0+X > locMaxWidth) || (Y0 + Y > locMaxHeight))
        return STERR_BADPARAMETER;
    
    // over sub-sample
    if(IncX > X || IncY > Y)
        return STERR_BADPARAMETER;
    
    // IncX should be a value to match with the end
    if(X%IncX !=0)
        return STERR_BADPARAMETER;
    
    if(Y%IncY !=0)
        return STERR_BADPARAMETER;
    
    // number of samples in X
    XSamples = X /IncX;
    YSamples = Y /IncY;
    
    if((XSamples*YSamples)%64!=0)
        return STERR_BADPARAMETER;
    
    // end check
    
    // update globals
    GlbWin->Height = YSamples;
    GlbWin->Width = XSamples;
    GlbWin->DeltaX = IncX;
    GlbWin->DeltaY = IncY;
    GlbWin->StartX = X0;
    GlbWin->StopX = X0+X-IncX;
    GlbWin->StartY = Y0;
    GlbWin->StopY = Y0+Y-IncY;
    
    // Write the values in the registries 
    iStatus = TC_SetWindowSize(GlbWin->StartX, GlbWin->StartY, GlbWin->StopX, GlbWin->StopY, IncX, IncY);
    
    return iStatus;  
}


//------------------------------------------------------------------------------------
/**
 * @brief The function writes the window size to the Chip registries, without changing
 * the values of the globals Width and Height.
 *
 * @param StartX		X coordinate of the window starting point
 * @param StartY		Y coordinate of the window starting point
 * @param StopX		X coordinate of the window ending point
 * @param StopY		Y coordinate of the window ending point
 * @param IncX		X sub sample 
 * @param IncY		Y sub sample
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_SENSOR_COMMUNICATION if a communication error has occurred
 * @return (other values) Status returned by the functions @ref OnSendCmdTableUSB, @ref WriteLogicalRegister
 */
//-------------------------------------------------------------------------------------
int TC_SetWindowSize(UINT StartX, UINT StartY, UINT StopX, UINT StopY, UINT IncX, UINT IncY)
{
    int StartCol_LSB, StartCol_MSB;
    int StopCol_LSB, StopCol_MSB;
    int StartRow_LSB, StartRow_MSB;
    int StopRow_LSB, StopRow_MSB;
    BYTE table[10];
    int iStatus;
    
    // check first if an error has already occurred
    if (Glbv.Prv.isCommunicationError)
    {
        return STERR_SENSOR_COMMUNICATION;
    }
    
    StartCol_LSB = StartX;		// assume sensor is less that 256 wide!
    StartCol_MSB = 0;
    
    StopCol_LSB = StopX;
    StopCol_MSB = 0;
    
    StartRow_LSB = (StartY & 0x00FF); 
    StartRow_MSB = ((StartY>>8) & 0x00FF);
    
    StopRow_LSB = (StopY & 0x00FF);
    StopRow_MSB = ((StopY>>8) & 0x00FF);

    // load the parameter for sending the command
    table[0] = (BYTE) StartCol_LSB;
    table[1] = (BYTE) StartCol_MSB;
    table[2] = (BYTE) StopCol_LSB;
    table[3] = (BYTE) StopCol_MSB;
    table[4] = (BYTE) IncX;
    table[5] = (BYTE) StartRow_LSB;
    table[6] = (BYTE) StartRow_MSB;
    table[7] = (BYTE) StopRow_LSB;
    table[8] = (BYTE) StopRow_MSB;
    table[9] = (BYTE) IncY;
    iStatus = OnSendCmdTableUSB( ST_TOUCH_SET_WINDOW, 0, table, 10);
    if(Check_USB_CommunicationError(&iStatus) != STERR_OK)
        return iStatus;
    
    return STERR_OK;
    
}

//------------------------------------------------------------------------------------
/**
 * @brief The function defines the correct window size for the current sensor
 *
 * @param SensorType		Sensor version (1A/1C/2A) - @ref sensorversions
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_BAD_DEVICE	if invalid sensor version
 */
//-------------------------------------------------------------------------------------
int InitWindowSize(BYTE SensorType)
{	
    switch (SensorType) {
        
    case S1A_SENSOR:
        GlbWin->MaxWidth=256;
        GlbWin->MaxHeight=360;
        break;
    case S1C_SENSOR:
        GlbWin->MaxWidth=256;
        GlbWin->MaxHeight=360;
        break;
    case S2A_SENSOR:
        GlbWin->MaxWidth=208;
        GlbWin->MaxHeight=288;
        break;
    case S2C_SENSOR:
        GlbWin->MaxWidth=208;
        GlbWin->MaxHeight=288;
        break;
    case FUTURE_SENSOR1:
        GlbWin->MaxWidth=256;
        GlbWin->MaxHeight=360;
        break;
    case FUTURE_SENSOR2:
        GlbWin->MaxWidth=256;
        GlbWin->MaxHeight=360;
        break;
    default:
        return STERR_BAD_DEVICE;
    }
    
    GlbWin->StartX	= 0; 
    GlbWin->StopX	= GlbWin->MaxWidth-1;
    GlbWin->StartY	= 0;
    GlbWin->StopY	= GlbWin->MaxHeight-1;
    GlbWin->Height	= GlbWin->MaxHeight;
    GlbWin->Width	= GlbWin->MaxWidth;
    GlbWin->DeltaX	= 1;
    GlbWin->DeltaY	= 1;
    
    return STERR_OK;
}



//-------------------------- Random number generator -----------------------------
/**
 * @brief Generate random data
 *
 * @param DataBuffer Buffer for acquired random data
 * @param BufferSize Size of data buffer
 *
 * @return @ref STERR_OK upon success
 * @return @ref STERR_ERROR for standard errors
 * @return @ref STERR_MEMORY_MALLOC_FAIL if memory allocation failure
 * @return (other values) Errors from internal functions
 */
int RandomDataGenerator(void *DataBuffer, int *BufferSize)
{
    unsigned char *img = NULL, *imgbin = NULL;
    const int sizeof_img = sizeof(unsigned char)*(XMAX*YMAX);
    const int sizeof_imgbin = sizeof(unsigned char)*(XMAX*YMAX/8);
    int i,j=0,k,maxj;
    int iStatus,grabStatus;
    int offset,gain,selbuf,freqas;
    int locWidth,locHeight,locWindowSize;
    WindowSettings wsize;
    const int ThreShold=128;
    
    // Set the window size
    SaveWindowSettings(&wsize);
    locWidth=locHeight=min(GlbWin->MaxWidth,GlbWin->MaxHeight);
    locWindowSize=locWidth*locHeight;
    iStatus = SetWindow(0, 0, locWidth, locHeight, 1, 1);
    if(iStatus !=STERR_OK)
        return iStatus;
    
    // read and save the values in registers
    iStatus = ReadLogicalRegister(OFFSET,&offset, Force);
    if(iStatus != STERR_OK)
        return iStatus;
    iStatus = ReadLogicalRegister(GAIN,&gain, NoForce);
    if(iStatus != STERR_OK)
        return iStatus;
    iStatus = ReadLogicalRegister(SELBUF,&selbuf, NoForce);
    if(iStatus != STERR_OK)
        return iStatus;
    iStatus = ReadLogicalRegister(FREQAS, &freqas, NoForce);
    if(iStatus != STERR_OK)
        return iStatus;
    
    iStatus = WriteLogicalRegister(OFFSET,35, NoForce);
    if(iStatus != STERR_OK)
        return iStatus;
    iStatus = WriteLogicalRegister(GAIN,7, NoForce);
    if(iStatus != STERR_OK)
        return iStatus;
    iStatus = WriteLogicalRegister(SELBUF,1, NoForce);
    if(iStatus != STERR_OK)
        return iStatus;
    iStatus = WriteLogicalRegister(FREQAS,7, Force);
    if(iStatus != STERR_OK)
        return iStatus;
    
    img = (unsigned char*)malloc(sizeof_img);
    if(img == NULL)
    {
        return STERR_MEMORY_MALLOC_FAIL;
    }
    imgbin = (unsigned char*)malloc(sizeof_imgbin);
    if(imgbin == NULL)
    {
        free(img);
        return STERR_MEMORY_MALLOC_FAIL;
    }

    // grab the image from the sensor
    grabStatus = GrabRawImage(img, locWindowSize);
    if(grabStatus == STERR_OK) 
	{   
        // build the random number
        memset(imgbin,0,sizeof_imgbin);
        maxj=(*BufferSize<sizeof_imgbin)?*BufferSize:sizeof_imgbin;
        for (i=0,j=0,k=0; i< sizeof_img && j<maxj;) {
            if (img[i]>ThreShold)
                imgbin[j] = imgbin[j]|(1<<(k%8));
            if ((++k%8)==0) j++;
            //			if ((i+1)<XMAX*XMAX)
            if ((i+1)<locWindowSize)
                i+=locWidth+1;		// to read the pixels on a diagonal
            else {
                // after 32 bytes, change image to lower the correlation
                grabStatus = GrabRawImage(img, locWindowSize);
                if(grabStatus != STERR_OK)
                    break;
                i=0;
            }
        }
    } 
    
    //	restore original register settings
    iStatus = WriteLogicalRegister(OFFSET, offset, NoForce);
    if(iStatus != STERR_OK)
        goto lbExit;
    iStatus = WriteLogicalRegister(GAIN, gain, NoForce);
    if(iStatus != STERR_OK)
        goto lbExit;
    iStatus = WriteLogicalRegister(SELBUF, selbuf, NoForce);
    if(iStatus != STERR_OK)
        goto lbExit;
    iStatus = WriteLogicalRegister(FREQAS, freqas, Force);
    if(iStatus != STERR_OK)
        goto lbExit;
    iStatus = RestoreWindowSettings(&wsize);
    if(iStatus != STERR_OK)
        goto lbExit;
    
    // load the output parameters 
    *BufferSize=j;
    memcpy(DataBuffer, (void *)imgbin, *BufferSize);
    
lbExit:
    if(img) free(img);
    if(imgbin) free(imgbin);

    if (grabStatus != STERR_OK)
        return grabStatus;
    else 
        return iStatus;
}



//--------------------------- Local Functions -------------------------------


//------------------------------------------------------------------------------------
/**
 * @brief The function initializes the TC registers
 *
 * @param SensorVersion		Sensor version - @ref sensorversions
 *
 * @return @ref STERR_OK upon success
 * @return (other values) Internal errors
 */
//-------------------------------------------------------------------------------------
int InitSensorRegisters(BYTE SensorVersion)
  {
      int iResult;
      
      //  set charge enable
      //  disable charge enable for windowing purpose!!!
      iResult = WriteLogicalRegister(CHENABLE,0, NoForce );
      if(iResult !=0)
          return iResult;
      
      // set USB Anchor Chip
      if(Glbv.Prv.rp->ReaderMode == READER_MODE_STD) {
          // set USB Anchor Chip to fast speed Acq.
          iResult = WriteLogicalRegister(XFER_SPEED, 1, NoForce );
          //		USB_SPEED = 1;
      }
      else {
          // set USB Anchor Chip to low speed Acq.
          iResult = WriteLogicalRegister(XFER_SPEED, 0, NoForce );
          //		USB_SPEED = 0;
      }
      if(iResult !=0)
          return iResult;
      
      iResult = WriteLogicalRegister(PERIODWIDTH,Glbv.Prv.PeriodWidth, NoForce);
      if(iResult !=0)
          return iResult;
      
      iResult = WriteLogicalRegister(CHARGEWIDTH,Glbv.Prv.ChargeWidth , NoForce);
      if(iResult !=0)
          return iResult;
      
      iResult = WriteLogicalRegister(RESETWIDTH, Glbv.Prv.ResetWidth, NoForce);
      if(iResult !=0)
          return iResult;
      
      // FreeRun
      iResult = WriteLogicalRegister(FREERUN, 1, NoForce);
      if(iResult != STERR_OK)
          return iResult;
      
      iResult = WriteLogicalRegister(FREQAS,6, NoForce);
      if(iResult !=0)
          return iResult;
      
      iResult = WriteLogicalRegister(GAIN,7, NoForce);
      if(iResult !=0)
          return iResult;
      
      iResult = WriteLogicalRegister(CHARGE,30,NoForce);
      if(iResult !=0)
          return iResult;
      
      iResult = WriteLogicalRegister(SIGNCHARGE,0,NoForce);
      if(iResult !=0)
          return iResult;
      
      //EIM: modify clock
      if(Glbv.Prv.rp->ReaderMode != READER_MODE_STD) {
          iResult = WriteLogicalRegister(CHKCLK,8,NoForce);
          if(iResult !=0)
              return iResult;
          
          iResult = WriteLogicalRegister(CLOCKOUTCNTR,1,NoForce);
          if(iResult !=0)
              return iResult;

          iResult = WriteLogicalRegister(GPIOCNT,2,NoForce);
          if(iResult !=0)
              return iResult;
      }
      else
      {
          iResult = WriteLogicalRegister(GPIOCNT,7,NoForce);
          if(iResult !=0)
              return iResult;
      }

      // Set TestFifo to 0xff, so can be checked at grabbing for reset event
      iResult = WriteLogicalRegister(TESTDATA,TC_STAT_NORMAL,Force);
      if(iResult !=0)
          return iResult;
      
      return STERR_OK;
      
  }
  
  
  
  //---------------------------------------------------------------------------
  /**
   * @brief Do digital gain compensation for the input image buffer for TCS1C sensors 
   *					
   * @param		Buffer		  Image buffer for adapting
   *
   * @param		BufferSize	Image buffer size
   */
  //---------------------------------------------------------------------------- 

  void DigitalGainCompensate(unsigned char *Buffer, unsigned long BufferSize) 
  {
      unsigned long i;
      
      if ((Glbv.Prv.TC_OverRideFlags&TC_DIG_GAIN_OVERRIDE)==0) {         
          for (i=0; i<BufferSize; i++){
              Buffer[i] = Glbv.Prv.D_gain[Buffer[i]];
          }
      }
  }
  

#define BAND_OFFSET_STEP 2  /**< @brief how much the offset values in the light and dark bands
                                 differ from the standard value */
//------------------------------------------------------------------------------------
/**
 * @brief The function loads the default setting table into the global settings.
 *        (shared by all sensor types)
 */
//-------------------------------------------------------------------------------------

void BuildDefaultTable(){
	int chrg,i;


    // move through the useful charge range 3 times
    // use different offset values in each 
    // move charge down then up then down to minimize charge steps
    // at the edges of the bands.

    // make sure setting 15 agrees with the "old" table

    // first move down through the charge range.
    i=0;
	for(chrg=45; chrg>=18; chrg-=3)
	{
		if(chrg<30) {
			Glbv.Prv.TCSettingTable[i].iChrg=chrg;
                        //Offset is -BAND_OFFSET_STEP in this band
			Glbv.Prv.TCSettingTable[i].iOffset=(int)(((double)chrg-7.0)/23.0*21.0 + 13.0)-BAND_OFFSET_STEP;
			Glbv.Prv.TCSettingTable[i].iGain=7;
			Glbv.Prv.TCSettingTable[i].iSignChrg=0;
		}
		else {
			Glbv.Prv.TCSettingTable[i].iChrg=chrg;
			Glbv.Prv.TCSettingTable[i].iOffset=(int)(((double)chrg-30.0)/22.0*15.0 + 34.0)-BAND_OFFSET_STEP;
			Glbv.Prv.TCSettingTable[i].iGain=7;
			Glbv.Prv.TCSettingTable[i].iSignChrg=0;
		}
        i++;
    }
    // now move back up through the charge range.
	for(chrg=15; chrg<=46; chrg+=3)
	{
		if(chrg<30) {
			Glbv.Prv.TCSettingTable[i].iChrg=chrg;
                        //Offset is "normal" in this band
			Glbv.Prv.TCSettingTable[i].iOffset=(int)(((double)chrg-7.0)/23.0*21.0 + 13.0);
			Glbv.Prv.TCSettingTable[i].iGain=7;
			Glbv.Prv.TCSettingTable[i].iSignChrg=0;
		}
		else {
			Glbv.Prv.TCSettingTable[i].iChrg=chrg;
			Glbv.Prv.TCSettingTable[i].iOffset=(int)(((double)chrg-30.0)/22.0*15.0 + 34.0);
			Glbv.Prv.TCSettingTable[i].iGain=7;
			Glbv.Prv.TCSettingTable[i].iSignChrg=0;
		}
        i++;
    }

    // now move back down through the charge range.
	for(chrg=45; chrg>=15; chrg-=3)
	{
		if(chrg<30) {
			Glbv.Prv.TCSettingTable[i].iChrg=chrg;
                        //Offset is +BAND_OFFSET_STEP in this band
			Glbv.Prv.TCSettingTable[i].iOffset=(int)(((double)chrg-7.0)/23.0*21.0 + 13.0)+BAND_OFFSET_STEP;
			Glbv.Prv.TCSettingTable[i].iGain=7;
			Glbv.Prv.TCSettingTable[i].iSignChrg=0;
		}
		else {
			Glbv.Prv.TCSettingTable[i].iChrg=chrg;
			Glbv.Prv.TCSettingTable[i].iOffset=(int)(((double)chrg-30.0)/22.0*15.0 + 34.0)+BAND_OFFSET_STEP;
			Glbv.Prv.TCSettingTable[i].iGain=7;
			Glbv.Prv.TCSettingTable[i].iSignChrg=0;
		}
        i++;
    }


	Glbv.Prv.TotalSettings = i;

}

int DetermineFingerType(unsigned char *image, int imgWidth, int imgHeight, int *fingContrast)
{
	double average, sigma;
	int peakPosition, peakValue, widthofHalfHeight;
	int histogram[254];
	int StartRow, EndRow, StartCol, EndCol;

	// get the finger window
	if(!CreateFingerWindow(image, imgWidth, imgHeight, &StartRow, &EndRow, &StartCol, &EndCol))
	{
	    StartRow = (int)ceil(imgHeight / FINGER_WINDOW_BOXAVG_FBRDR_ROW);
	    EndRow = ((imgHeight - 1) - (int)ceil(imgHeight / FINGER_WINDOW_BOXAVG_FBRDR_ROW));
	    StartCol = (int)ceil(imgWidth / FINGER_WINDOW_BOXAVG_FBRDR_COL);
	    EndCol = ((imgWidth - 1) - (int)ceil(imgWidth / FINGER_WINDOW_BOXAVG_FBRDR_COL));
	}

	// compute the image histogram & finger contrast
	ComputeImageHistogram(image, imgWidth, imgHeight, StartRow, EndRow, StartCol, EndCol, &average, &sigma, &peakPosition, &peakValue, &widthofHalfHeight, histogram);
	*fingContrast = GetHistogramContrastBetween(histogram, FAINTFNG_CONTRAST_PERCENT_EX);

    // decide whether to use static or dynamic digital gain
    if(Glbv.Prv.UseDynamicDigitalGainForFaintFinger)
    {
	    // faint finger or no finger?
	    if(*fingContrast < (int)((Glbv.Prv.rp->SensorType==SENSOR_TYPE_C_STEELCOAT)?(Glbv.Prv.DynamicSCNoFingerThreshold):(Glbv.Prv.DynamicNoFingerThreshold)))
	    {
		    return STERR_NO_FINGER;
		}
	    else if(*fingContrast < (int)((Glbv.Prv.rp->SensorType==SENSOR_TYPE_C_STEELCOAT)?(Glbv.Prv.DynamicSCFaintFingerThreshold):(Glbv.Prv.DynamicFaintFingerThreshold)))
	    {
		    return STERR_FAINT_FINGER;
		}
    }
    else
    {
	    // faint finger?
	    if(*fingContrast < (int)((Glbv.Prv.rp->SensorType==SENSOR_TYPE_C_STEELCOAT)?(Glbv.Prv.StaticSCFaintFingerThreshold):(Glbv.Prv.StaticFaintFingerThreshold)))
	    {
		    return STERR_FAINT_FINGER;
		}
    }

	return STERR_OK;
}

//------------------------------------------------------------------------------------
/**
 * @brief Calculates the location of the finger window in the image buffer.
 *
 * A finger window is a window within the image buffer that encompasses the entire grabbed 
 * finger to some degree of tolerance.  Calculating this requires the following steps for 
 * both rows and columns:
 *
 * 1. The row/col average array is computed.
 * 2. The row/col box average array is computed from the row/col average array to make 
 *    its curves smoother and more accurate (respectively).  
 * 3. Lastly, two points are marked on the row/col box average curve to mark the begining 
 *    and ending row/col points of the grabbed finger (respectively).  
 *
 * The finger window is created from the intersection of these 2 calculated rows and 2 
 * calculated cols.  Before returning, the finger window dimensions are checked to make sure 
 * they form a valid rectangle.
 *
 *        Image Buffer
 *  rows
 *    |ooooooooooooooo       ooo denotes contents of image buffer not in finger window
 *    +ooo#~~~~~~~#ooo       xxx denotes contents of image buffer in finger window
 *    |ooo~xxxxxxx~ooo  <--- grabbed finger should be majorily inside the finger window
 *    +ooo#~~~~~~~#ooo       + points indicate the 4 points calculated by the above 3 steps
 *    |ooooooooooooooo       # denotes intersection of the 4 calculated rows and cols
 *     ---+-------+--- cols  ~ denotes borders of finger window
 *
 * @param image             The image buffer containing the grabbed finger
 * @param imgWidth          The width of the image buffer 
 * @param imgHeight         The height of the image buffer 
 * @param StartRow          The row that borders the top of the resulting finger window
 * @param EndRow            The row that borders the bottom of the resulting finger window
 * @param StartCol          The column that borders the left of the resulting finger window
 * @param EndCol            The colun that borders the right of the resulting finger window
 *
 * @return TRUE  If finger window dimensions form a valid rectangle
 * @return FALSE Otherwise
 */
//------------------------------------------------------------------------------------

static BOOL CreateFingerWindow(unsigned char *image, int imgWidth, int imgHeight, int *StartRow, int *EndRow, int *StartCol, int *EndCol)
{
	unsigned int *avg;
	unsigned int *boxavg;
	int x, y, i;
	int boxavgoff_row, boxavgoff_col;

	//calculate the box average offset for row & col
	boxavgoff_row = (int)ceil(imgHeight / FINGER_WINDOW_BOXAVG_FBRDR_ROW);
	boxavgoff_col = (int)ceil(imgWidth / FINGER_WINDOW_BOXAVG_FBRDR_COL);

	//set defaults to negative values to ensure they get moved
	*StartRow = *EndRow = *StartCol = *EndCol = -1;

	//compute the column average
	avg = (unsigned int*)malloc(imgWidth*sizeof(unsigned int));
	if(avg == NULL) return FALSE;
	memset(avg, 0, imgWidth*sizeof(unsigned int));
	for(x=0; x<imgWidth; x++)
	{
		for(y=0; y<imgHeight; y++)
			avg[x] += image[(y*imgWidth)+x];
		avg[x] /= imgHeight;
	}
	//compute the column box average
	boxavg = (unsigned int*)malloc(imgWidth*sizeof(unsigned int));
	if(boxavg == NULL) { free(avg); return FALSE; }
	memset(boxavg, 0, imgWidth*sizeof(unsigned int));
	for(x=boxavgoff_col; x<(imgWidth-boxavgoff_col); x++)
	{
		for(i=x; i<x+boxavgoff_col; i++)
			boxavg[x] += avg[i];
		if(boxavgoff_col != 0)
			boxavg[x] /= boxavgoff_col;
	}
	//find the col boundaries
	for(x=boxavgoff_col; x<(imgWidth-boxavgoff_col); x++)
	{
		if(abs(IMG_AVGBKGND_VALUE - boxavg[x]) > (int)((Glbv.Prv.rp->SensorType==SENSOR_TYPE_C_STEELCOAT)?(Glbv.Prv.SCFingerTriggerThreshold):(Glbv.Prv.FingerTriggerThreshold)))
		{
			*StartCol = x;
			break;
		}
	}
	for(x=(imgWidth-boxavgoff_col-1); x>=boxavgoff_col; x--)
	{
		if(abs(IMG_AVGBKGND_VALUE - boxavg[x]) > (int)((Glbv.Prv.rp->SensorType==SENSOR_TYPE_C_STEELCOAT)?(Glbv.Prv.SCFingerTriggerThreshold):(Glbv.Prv.FingerTriggerThreshold)))
		{
			*EndCol = x;
			break;
		}
	}
	free(avg);
	free(boxavg);

	//compute the row average
	avg = (unsigned int*)malloc(imgHeight*sizeof(unsigned int));
	if(avg == NULL) return FALSE;
	memset(avg, 0, imgHeight*sizeof(unsigned int));
	for(y=0; y<imgHeight; y++)
	{
		for(x=0; x<imgWidth; x++)
			avg[y] += image[(y*imgWidth)+x];
		avg[y] /= imgWidth;
	}
	//compute the row box average
	boxavg = (unsigned int*)malloc(imgHeight*sizeof(unsigned int));
	if(boxavg == NULL) { free(avg); return FALSE; }
	memset(boxavg, 0, imgHeight*sizeof(unsigned int));
	for(y=boxavgoff_row; y<(imgHeight-boxavgoff_row); y++)
	{
		for(i=y; i<y+boxavgoff_row; i++)
			boxavg[y] += avg[i];
		if(boxavgoff_row != 0)
			boxavg[y] /= boxavgoff_row;
	}
	//find the row boundaries
	for(y=boxavgoff_row; y<(int)(imgHeight-boxavgoff_row); y++)
	{
		if(abs(IMG_AVGBKGND_VALUE - boxavg[y]) > (int)((Glbv.Prv.rp->SensorType==SENSOR_TYPE_C_STEELCOAT)?(Glbv.Prv.SCFingerTriggerThreshold):(Glbv.Prv.FingerTriggerThreshold)))
		{
			*StartRow = y;
			break;
		}
	}
	for(y=(imgHeight-boxavgoff_row-1); y>=boxavgoff_row; y--)
	{
		if(abs(IMG_AVGBKGND_VALUE - boxavg[y]) > (int)((Glbv.Prv.rp->SensorType==SENSOR_TYPE_C_STEELCOAT)?(Glbv.Prv.SCFingerTriggerThreshold):(Glbv.Prv.FingerTriggerThreshold)))
		{
			*EndRow = y;
			break;
		}
	}
	free(avg);
	free(boxavg);

	//validate rectangle
	if((*StartRow < 0) || (*EndRow < 0) || (*StartCol < 0) || (*EndCol < 0) //make sure all dimensions changed
		|| (*EndRow <= ((*StartRow)+(int)((Glbv.Prv.rp->SensorType==SENSOR_TYPE_C_STEELCOAT)?(Glbv.Prv.SCFingerBoxMinThreshold):(Glbv.Prv.FingerBoxMinThreshold)))) //make sure not line, inverted rectangle, or very small
		|| (*EndCol <= ((*StartCol)+(int)((Glbv.Prv.rp->SensorType==SENSOR_TYPE_C_STEELCOAT)?(Glbv.Prv.SCFingerBoxMinThreshold):(Glbv.Prv.FingerBoxMinThreshold)))))   
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------------
/**
 * @brief Computes the histogram of the specified subwindow of the image (borders included)
 *
 * @param image             The image buffer from which to compute the histogram
 * @param imgWidth          The width of the image buffer 
 * @param imgHeight         The height of the image buffer 
 * @param StartRow          The row that borders the top of the target subwindow
 * @param EndRow            The row that borders the bottom of the target subwindow
 * @param StartCol          The column that borders the left of the target subwindow
 * @param EndCol            The column that borders the right of the target subwindow
 * @param average           Average pixel value in the image
 * @param sigma             The spread of pixel values from the average (higher means greater)
 * @param peakPosition      The pixel value with the highest number of occurrence (0-253)
 * @param peakValue         The number of pixels having value equal to peakPosition
 * @param widthofHalfHeight The distance in-between the highest curve at the half value
 * @param histogram         The resulting histogram (0-253)
 */
//------------------------------------------------------------------------------------

static void ComputeImageHistogram(unsigned char *image, int imgWidth, int imgHeight, int StartRow, 
     int EndRow, int StartCol, int EndCol, double *average, double *sigma, int *peakPosition, 
     int *peakValue, int *widthofHalfHeight, int *histogram)
{
  int i, j, k, numPix;
  double numAve;
  int firstCoor=0, secondCoor=253;
  *average=0;
  *sigma=0;
  *peakPosition=-1;
  *peakValue=-1;
  numPix=0;
  numAve=0;
  for(k=0; k<254; k++) histogram[k]=0;  
  for(j=StartRow; j<=EndRow; j++) {
    for(i=StartCol; i<=EndCol; i++) {
      histogram[image[j*imgWidth+i]]++;
      *average += image[j*imgWidth+i];
      numPix++;
    }
  }
  *average /= numPix;
  for(j=StartRow; j<=EndRow; j++) {
    for(i=StartCol; i<=EndCol; i++) {
      *sigma += (image[j*imgWidth+i]-(*average))*(image[j*imgWidth+i]-(*average));
    }
  }
  *sigma = sqrt((*sigma)/(numPix-1));

  for(k=0; k<254; k++) {
    if (histogram[k]>(*peakValue)) {
      *peakValue=histogram[k];
      *peakPosition=k;
    }
  }
  for (i=0; i<*peakPosition; i++) {
    if((histogram[i]<=*peakValue/2)&&(histogram[i+1]>=*peakValue/2)) {
      firstCoor=i;
      break;
    }
  }
  for (i=*peakPosition; i<253; i++) {
    if((histogram[i+1]<=*peakValue/2)&&(histogram[i]>=*peakValue/2)) 
      secondCoor=i+1;
  }

  *widthofHalfHeight=secondCoor-firstCoor;
}

//------------------------------------------------------------------------------------
/**
 * @brief Computes the contrast of the image histogram excluding the specified percentage 
 * range on both sides (high and low).
 *
 * @param histogram         The image histogram (0-253)
 * @param percentTH         Algorithm will mark the high/low position in the histogram with 
 *                          this percentage of total pixels above/below it (respectively)
 *
 * @return The resulting contrast of the image histogram
 */
//------------------------------------------------------------------------------------

static int GetHistogramContrastBetween(int *histogram, double percentTH)
{
	int i, lowTH, highTH;
	unsigned long pixcount, totalPixels;

	//calculate the total number of pixels in the histogram
	totalPixels = 0;
	for(i=0; i<254; i++)
		totalPixels += histogram[i];

	//find low point
	pixcount = 0;
	for(lowTH=0; lowTH<254; lowTH++)
	{
		pixcount += histogram[lowTH];
		if(pixcount >= (unsigned long)(totalPixels * percentTH))
			break;
	}
	//find high point
	pixcount = 0;
	for(highTH=253; highTH>=0; highTH--)
	{
		pixcount += histogram[highTH];
		if(pixcount >= (unsigned long)(totalPixels * percentTH))
			break;
	}

	return (highTH - lowTH);
}

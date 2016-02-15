//---------------------------------------------------------------------------------------------------------
// sample.c - Demonstrates how to use the PerfectPrint(TM) API to grab an image from the TouchChip(TM)
//---------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <unistd.h>

// Must include header file for PerfectPrint(TM) - types.h must be in same directory
#include "perfectprintapi.h"

#define IMGFILE "img.pgm"          // Name of image file to put grabbed image
#define ALGOID "04E8B2723421EA84"  // Using hardcoded AlgoID

//void Sleep(int ms)
//{
//    usleep(ms*1000);
//}

int main(int argc, char *argv[]) 
{
  // Return code of PerfectPrint(TM) functions
  int iStatus = STERR_OK;

  //////////////////////////////////////////////////////////////////////
  // Preparing the chip.                                              //
  // Must be called in this order: STInitialize, STAuthentify, STOpen //
  //////////////////////////////////////////////////////////////////////

  // Initializing communication with the TouchChip(TM)
  // int STInitialize(HANDLE hFingerPresent, HANDLE hFingerLeft);
  // Currently the hFingerPresent and hFingerLeft parameters are not supported (use NULLs)
  iStatus = STInitialize(NULL, NULL);
  if(iStatus != STERR_OK)
  {
    printf("There have been problems in initializing the TouchChip(TM)\nReturn Code (%d)\n", iStatus);
    return iStatus;
  }

  // Declare the buffers for the license verification.
  unsigned char *myAlgoID=(unsigned char*)ALGOID;   // AlgoID should hold the actual algorithm id.
  unsigned char PassPhrase[16];                     // Maximum size for PassPhrase buffer is 16

  // License verification
  // int STAuthentify(unsigned char *AlgoId, unsigned char *PassPhrase);
  // Checks whether AlgoId contains a valid algoirthm id, the correspondent PassPhrase is returned.
  // STAuthentify must be called after STInitialize and before STOpen.
  iStatus = STAuthentify(myAlgoID, PassPhrase);
  if(iStatus != STERR_OK)
  {
    STTerminate();
    printf("Invalid license\nReturn Code (%d)\n", iStatus);
    return iStatus;
  }

  // Declare the data structure for the STOpen function
  // The SensorDescr struct contains the sensor's version and image width & height.
  SensorDescr SensorInfo;

  // Opening the TouchChip(TM) for access
  // int STOpen(&SensorInfo);
  // STOpen must be called after STAuthentify.
  iStatus = STOpen(&SensorInfo);
  if(iStatus != STERR_OK)
  {
    STTerminate();
    printf("There have been problems in opening the TouchChip(TM)\nReturn Code (%d)\n", iStatus);
    return iStatus;
  }

  /////////////////////////////////////////////////
  // Now the chip is ready to accept any command //
  /////////////////////////////////////////////////

  // Declare buffer used to store the image grabbed.
  // Prepare for the largest image possible (XMAX & YMAX are maximum sensor window size).
  unsigned char image[XMAX * YMAX];

  // Example of grabbing a FULL SIZE fingerprint image using the best quality settings
  // int STSetWindow(int x0, int y0, int x, int y, int delta_x, int delta_y);
  iStatus = STSetWindow(0, 0, SensorInfo.ImageWidth, SensorInfo.ImageHeight, 1, 1);   //The Standard Full Window
  if(iStatus != STERR_OK)
  {
    STClose();
    STTerminate();
    printf("There have been problems in setting the window\nReturn Code(%d)\n", iStatus);
    return iStatus;
  }

  // Grab a full size image using the best settings & print to file
  // int STGrabBestSett(unsigned char *image, unsigned long WIDTH*HEIGHT);
  printf("Put your finger on the device ..\n");
  Sleep(5000);
  iStatus = STGrabBestSett(image, (SensorInfo.ImageWidth)*(SensorInfo.ImageHeight));
  
  // If you do not want to save faint finger images (~fingerprint image contrast between 20-50lsb) then comment "iStatus != STERR_FAINT_FINGER" in the following condition
  if(iStatus != STERR_OK && iStatus != STERR_FAINT_FINGER)
  {
    STClose();
    STTerminate();
    printf("There have been problems in grabbing an image\nReturn Code (%d)\n", iStatus);
    return iStatus;
  }
  else
  {
    //write image to file
    FILE *imgFile;
    int ipixel;
    imgFile = fopen(IMGFILE, "wb+");
    if (imgFile != NULL)
    {
      fprintf(imgFile, "P2\n%d %d\n255\n", SensorInfo.ImageWidth, SensorInfo.ImageHeight);
      for (ipixel=0; ipixel<((SensorInfo.ImageWidth)*(SensorInfo.ImageHeight)); ipixel++)
      {
        fprintf(imgFile, "%3d ", image[ipixel]);
        if (ipixel && ((ipixel%16)==0)) fprintf(imgFile, "\n");
      }
      fclose(imgFile);
    }
  }

  printf("Remove your finger ..\n");
  Sleep(5000);


  //
  //
  // Do something with this image
  // for example extract a template
  // ..
  //
  //


  /////////////////////////
  // Disconnect the chip //
  /////////////////////////

  // Resets and shuts down the sensor - TouchChip(TM).
  // STClose must be called once for each call to STOpen.
  // int STClose();
  iStatus = STClose();
  if(iStatus != STERR_OK)
  {
    STTerminate();
    printf("There have been problems in closing the TouchChip(TM)\nReturn Code (%d)\n", iStatus);
    return iStatus;
  }

  // Release the sensor - TouchChip(TM)
  // STTerminate must be called once for each call to STInitialize.
  // int STTerminate();
  iStatus = STTerminate();  
  if(iStatus != STERR_OK)
  {
    printf("There have been problems in closing the TouchChip(TM)\nReturn Code (%d)\n", iStatus);
    return iStatus;
  } 

  return iStatus;
}

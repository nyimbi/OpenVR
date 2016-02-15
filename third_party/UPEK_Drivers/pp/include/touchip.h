/**
 * @file touchip.h
 * TouchChip Functions
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef TOUCHIP_H_
#define TOUCHIP_H_


//-------------------------------- Constants --------------------------------
/**
 * @defgroup mosaicparams Mosaic Parameters
 * Size for small images in the GrabMultipleImages
 * @{
 */
#define MOSAIC_WIDTH                    32
#define MOSAIC_HEIGHT                   32
/** @} */

/**
 * @defgroup grabtypes Grab Types
 * @{
 */
#define GRAB_TYPE_SETTINGS_GRAB         1   ///< Cycles through the settings table after every 1024 pixels
#define GRAB_TYPE_NEW_GRAB              2   ///< Packet count to eliminate dangling packet
/** @} */

/**
 * @defgroup imgconsts Image Constants
 * @{
 */
#define BG_AVG_PIX_VAL                  190
/** @} */

/**
 * @defgroup predefinedgains Digital Gain Defines
 * Predefined gains for Digital gain
 * @{
 */
#define GAIN_FOURTH                     0x08
#define GAIN_HALF                       0x10
#define GAIN_ONE                        0x20
#define GAIN_TWO                        0x40
#define GAIN_THREE                      0x60
#define GAIN_THREEANDAHALF              0x70
#define GAIN_FOUR                       0x80
                                        // you get the picture, add them up for fractional values
/** @} */

/**
 * @defgroup fingtypecalc Finger Type Calculation Defines
 * @{
 */
#define FINGER_WINDOW_BOXAVG_FBRDR_ROW  18  ///< The fraction of image border not used in computing the row box average of an image for finger window calculation
#define FINGER_WINDOW_BOXAVG_FBRDR_COL  16  ///< The fraction of image border not used in computing the col box average of an image for finger window calculation
#define FINGER_WINDOW_FDET_TOLERANCE    20  ///< The tolerated difference amount between a background pixel and finger pixel
#define IMG_AVGBKGND_VALUE              253 ///< The designated pixel value of "the background" (used to differentiate between finger and background pixels)
#define FAINTFNG_CONTRAST_PERCENT_EX    0.1 ///< Percentage of pixels not used on either side of histogram when measuring the faint finger contrast
#define FAINTFNG_CONTRAST_TH            50  ///< Fingers with contrast below this value are considered "faint/weak"
/** @} */

/**
 * @defgroup stableconsts Stability Constants
 * @{
 */
#define MAX_STABILITY_CHECKS            4
/** @} */



//--------------------------- Function prototypes ---------------------------
extern int GrabImage(unsigned char *Buffer, unsigned long BufferSize);
extern int GrabMultipleImages(unsigned char *Buffer, unsigned long BufferSize);

extern int InitSensorRegisters(BYTE SensorVersion);
extern int InitWindowSize(BYTE SensorType);
extern void InitDigitalGainTable(BYTE gain1, BYTE gain2, BYTE offsetparm, BYTE *pDigGain);
extern void BuildDefaultTable();
extern int TC_SetWindowSize(UINT StartX, UINT StartY, UINT StopX, UINT StopY, UINT IncX, UINT IncY);

extern int RandomDataGenerator(void *DataBuffer, int *BufferSize);

extern void InterpolateMissingPixels(BYTE *ImgBuff, WindowSettings *p_WinSetting);
extern void DigitalGainCompensate(unsigned char *Buffer, unsigned long BufferSize); 
extern int DetermineFingerType(unsigned char *image, int imgWidth, int imgHeight, int *fingContrast);

#endif  //#ifndef TOUCHIP_H_

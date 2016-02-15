/**
 * @file fingerdetect.h
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef FINGERDETECT_H_
#define FINGERDETECT_H_


//-------------------------------- Constants --------------------------------
/**
 * @defgroup fdthresholds Finger Detect Thresholds
 * Thresholds for the finger detect
 * @{
 */
#define TH_QUAL         0.1
#define TH_CONTR        60.0
#define TH_COUNT        2
#define TH_QUAL_MIN     0.06
#define TH_CONTR_MIN    30
#define TH_COUNT_MIN    1
#define WOFFSET         32
#define HOFFSET         32
/** @} */

/**
 * @defgroup pixelrepair Pixel Repair Parameters
 * Define parameters for the pixel repair
 * @{
 */
#define FD_WIDTH        16  ///< Should be around 16
#define FD_HEIGHT       16  ///< Should be around 16
/** @} */


//--------------------------- Function prototypes ---------------------------
extern int FingerDetect(UCHAR *Buffer, int Width, int Height, int columndpi, int rowdpi, BOOL *isFingerPresent, BOOL *isBadQualityFinger);

#endif  // #ifndef FINGERDETECT_H_

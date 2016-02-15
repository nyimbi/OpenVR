/**
 * @file fingerdetect.c
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

//-------------------------- Function definitions ---------------------------

/**
 *  @brief Finger detection switching function
 *
 *  Uses old or new method, depending on the override flag setting.
 *
 *  @param img                          Image to detect finger on
 *  @param width                        Width of image
 *  @param height                       Height of image
 *  @param columndpi                    Column DPI of image
 *  @param rowdpi                       Row DPI of image
 *  @param isFingerPresent              Is finger detected in image? (returned)
 *  @param isBadQualityFinger           Is detected finger of bad quality? (returned 
 *                                      and only useful if finger is present)
 *
 *  @see FingerDetect_TCAlgo
 *  @see FingerDetect_83
 *
 *  @return Values retured from internal functions
 */
int FingerDetect(UCHAR *img, int width, int height, int columndpi, int rowdpi, BOOL *isFingerPresent, BOOL *isBadQualityFinger)
{
    int retval;

    retval = STERR_OK;
    *isFingerPresent = TRUE; 
    *isBadQualityFinger = FALSE;

    return retval;
}

/**
 * @file qualeng.h
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
#ifndef QUALENG_H_
#define QUALENG_H_


//--------------------------- Function prototypes ---------------------------
// Get/Set setting functions for grab image
extern int GetBestSettings();
extern int GetSelSettings(int SelSett);

// Save and restore setting functions
extern double GetQuality(unsigned char *ImageBuff, int Width, int Height);
extern void AdaptSettings(setting *TCsett, int NumSettings, GainOffsetStruct *gain_offset, int SensorType);

#endif  // #ifndef QUALENG_H_

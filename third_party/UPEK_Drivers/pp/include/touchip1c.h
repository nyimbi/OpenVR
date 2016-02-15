/**
 * @file touchip1C.h
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef TOUCHIP1C_H_
#define TOUCHIP1C_H_


//-------------------------------- Constants --------------------------------
/**
 * @defgroup tcs1cnominalvalues TCS1C Sensor Nominal Values
 * Nominal values for the setting table
 * @{
 */
#define S1C_GAIN_RO     2.050
#define S1C_OFF_RO      -0.017
#define S1C_GAIN_PIX    0.645
#define S1C_OFF_PIX     0.126
/** @} */


//--------------------------- Function prototypes ---------------------------
void InitSettingsSensor1C();

#endif  // #ifndef TOUCHIPH1_H_

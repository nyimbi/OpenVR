/**
 * @file touchip2C.h
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef TOUCHIP2C_H_
#define TOUCHIP2C_H_


//-------------------------------- Constants --------------------------------
/**
 * @defgroup tcs2cnominalvalues TCS2C Sensor Nominal Values
 * Nominal values for the setting table
 * @{
 */
#define S2C_GAIN_RO     2.050
#define S2C_OFF_RO      -0.017
#define S2C_GAIN_PIX    0.645
#define S2C_OFF_PIX     0.126
/** @} */


//--------------------------- Function prototypes ---------------------------
void InitSettingsSensor2C();

#endif  // #ifndef TOUCHIP2C_H_

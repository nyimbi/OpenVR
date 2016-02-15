/**
 * @file touchip1A.h
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef TOUCHIP1A_H_
#define TOUCHIP1A_H_


//-------------------------------- Constants --------------------------------
/**
 * @defgroup h1sensorparms H1 Sensor Parameters
 * Definitions for isH1Sensor() function
 * @{
 */
#define LOW_OFFSET      0
#define HIGH_OFFSET     7
#define H0_AVERAGE      87
#define NUMPIX          64
/** @} */

/**
 * @defgroup tcs1anominalvalues TCS1A Sensor Nominal Values
 * Nominal values for the setting table
 * @{
 */
#define S1A_GAIN_RO     2.050
#define S1A_OFF_RO      -0.017
#define S1A_GAIN_PIX    0.645
#define S1A_OFF_PIX     0.126
/** @} */


//--------------------------- Function prototypes ---------------------------
void InitSettingsSensor1A();


#endif  // #ifndef TOUCHIPH1_H_

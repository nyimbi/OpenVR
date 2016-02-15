/**
 * @file Readout_constants.h
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef _READOUT_CONSTANTS_H_
#define _READOUT_CONSTANTS_H_


//-------------------------------- Constants --------------------------------
/**
 * @defgroup readoutconsts Readout Constants
 * @{
 */
#define OFFSET_BASELINE     (OFFSET_lsb*25.0 + OFFSET_DAC_offset)
#define CHARGE_BASELINE     (0.0 + CHARGE_DAC_offset)
#define REFL                2.0
#define REFH                (((5.0-REFL)*2.0/3.0) + REFL)
#define OFFSET_lsb          (5.0/128.0)
#define CHARGE_lsb          (5.0/128.0*2.0)
#define ADC_lsb             ((REFH-REFL)/255.0)
#define OFFSET_DAC_offset   0.0
#define CHARGE_DAC_offset   0.0
#define OFFSET_reference    0.0
/** @} */


#endif  // #ifndef _READOUT_CONSTANTS_H_

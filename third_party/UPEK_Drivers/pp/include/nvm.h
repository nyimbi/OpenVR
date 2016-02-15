/**
 * @file nvm.h
 * I/O functions to read/write NVM records
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef NVM_H_
#define NVM_H_


//-------------------------------- Constants --------------------------------
/**
 * @defgroup nvmparams NVM Parameters
 * @{
 */
#define NVM_FORMAT_VALID        0xEE    ///< Value stored in NVM validity flag when EEPROM format is valid - @ref _NVM_HEADER
#define VALIDITY_FLAG           0xA5    ///< Value of the validity field in the repair record
/**  @} */


//--------------------------- Function prototypes ---------------------------
extern int NVM_ReadLineRepairInfo(int *BadRow, int *BadCol);

extern int NVM_ReadPixRepairInfo(BadPix *psBP);

extern int NVM_ReadGainOffInfo(BYTE *GainOff);

extern int NVM_ReadGradientInfo(PGradientProfile p_gp);

extern int NVM_ReadReaderProfile(PReaderProfile p_rp, BOOL loadEIMGrad);


extern int InitNVMDir(NVM_DIR *pnvmd);
extern int OverrideGlobals(void);

#endif  // #ifndef NVM_H_

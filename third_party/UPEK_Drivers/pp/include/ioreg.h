/**
 * @file ioreg.h
 * I/O functions to read/write TC registers and NVM records
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef IOREG_H_
#define IOREG_H_


//-------------------------------- Constants --------------------------------
/**
 * @defgroup ioregparams I/O Register Parameters
 * @{
 */
#define MAX_WRITING_BLOCKS      25      ///< Max number of blocks in a multiple write
#define MAX_READING_BLOCKS      58      ///< Max number of blocks in a multiple read
/** @} */


//--------------------------- Function prototypes ---------------------------
extern int USB_Write_Register_Table(int Num_Regs, BYTE *reg_table);
extern int LoadRegTable();

#endif  // #ifndef IOREG_H_

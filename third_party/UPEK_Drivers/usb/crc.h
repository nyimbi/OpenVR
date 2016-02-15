/**
 * @file crc.h
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */

#ifndef __CRC_H__
#define __CRC_H__

#include "usbtypes.h"

uint16 ComputeCrc(uint8 *Buffer, uint32 Size, uint16 InitVal);

void ComputePseudoCrc32(uint8 *Buffer, uint32 Size, uint16 InitVal[2], uint8 StartEven);

uint8 ComputeCrc8(uint8 *Buffer, uint32 Size, uint8 InitVal);

#endif  // __CRC_H__

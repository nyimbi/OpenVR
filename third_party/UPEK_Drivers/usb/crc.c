/**
 * @file crc.c
 * CRC V41 implementation
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#include <Tdbg.h>

TDBG_DEFINE_AREA(Crc)

#include "crc.h"

uint16 ComputeCrc(uint8 *Buffer, uint32 Size, uint16 InitVal) {
/*     ======================================================
       Computes CRC V41 for given Buffer and Size.
*/
    uint16 Crc = InitVal;

    TTRACE_ENTER(Crc, TDBG_TRACE, ("ComputeCrc(%p, %ld, %x)", Buffer, Size, InitVal));
    TASSERT(Buffer != NULL);

    if (Buffer != NULL) {
        for( ; Size > 0; --Size ) {
            Crc = (uint16)((uint8)(Crc >> 8) | (Crc << 8));
            Crc ^= (uint8)*Buffer++;
            Crc ^= (uint8)(Crc & 0xff) >> 4;
            Crc ^= (Crc << 8) << 4;
            Crc ^= ((Crc & 0xff) << 4) << 1;
        }
    }
    TTRACE_LEAVE(("ComputeCrc() -> %x", Crc));
    return(Crc);
}

void ComputePseudoCrc32(uint8 *Buffer, uint32 Size, uint16 InitVal[2], uint8 StartEven) {
/*   ==================================================================================
     Computes CRC V41 for given even and old bytes separately.
*/
    uint16 *crc = InitVal;

    TTRACE_ENTER(Crc, TDBG_TRACE, ("ComputePseudoCrc32(%p, %lu, [%x:%x], %d)", Buffer, Size, InitVal[0], InitVal[1], StartEven));
    TASSERT(Buffer != NULL);

    if (Buffer != NULL) {
        if (!StartEven) {
            ++crc;
        }
        for( ; Size > 0; --Size ) {
            *crc = (uint16)((uint8)(*crc >> 8) | (*crc << 8));
            *crc ^= (uint8)*Buffer++;
            *crc ^= (uint8)(*crc & 0xff) >> 4;
            *crc ^= (*crc << 8) << 4;
            *crc ^= ((*crc & 0xff) << 4) << 1;

            if (++crc > &InitVal[1]) {
                crc = InitVal;
            }
        }
    }
    TTRACE_LEAVE(("ComputePseudoCrc32() [%x:%x]", InitVal[0], InitVal[1]));
}

uint8 ComputeCrc8(uint8 *Buffer, uint32 Size, uint8 InitVal) {
/*    ======================================================
      Computes 8-bit CRC for given Buffer and Size. Use shared crc.c version
      when moved out from XL. Pseudo-CRC, rewrite when have time.
*/
    uint8 Crc = InitVal;

    TTRACE_ENTER(Crc, TDBG_TRACE, ("ComputeCrc8(%p, %ld, %x)", Buffer, Size, InitVal));

    if (TVERIFY(Buffer != NULL)) {
        for( ; Size > 0; --Size ) {
            Crc = (uint8)((uint8)(Crc >> 4) | (Crc << 4));
            Crc ^= (uint8)*Buffer++;
            Crc ^= (uint8)(Crc & 0xf) >> 2;
            Crc ^= (Crc << 4) << 2;
            Crc ^= ((Crc & 0xf) << 2) << 1;
        }
    }
    TTRACE_LEAVE(("ComputeCrc8() -> %x", Crc));
    return(Crc);
}


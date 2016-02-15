/**
 * @file crypt.h
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef _CRYPT_H_
#define _CRYPT_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------- Function prototypes ---------------------------
/**
 *  @brief Gets the contact information
 *
 *  @param contact Contact information (returned).  Must be preallocated 
 *                 in memory to provide a valid pointer (>= 16 bytes).
 *
 *  @return 0 upon success
 */
int __stdcall getContactInformation(unsigned char *contact);

/**
 *  @brief Gets the software license
 *
 *  @param license Software license (returned).  Must be preallocated 
 *                 in memory to provide a valid pointer (>= 20 bytes).
 *
 *  @return 0 upon success
 */
int __stdcall getSoftwareLicense(unsigned char *license);

/**
 *  @brief Encrypt data in buffer using specified key
 *
 *  @param key         Identifier for encryption key to use
 *  @param clear_text  Data buffer to encrypt
 *  @param numbytes    Number of bytes in buffer to encrpyt
 *  @param isPresetKey Is @a key a preset key?
 *
 *  @return 0 upon success
 */
int __stdcall crypt(WORD key, BYTE *clear_text, BYTE numbytes, BOOL isPresetKey);

/**
 *  @brief Convert data in buffer from characters to numbers
 *
 *  @param buff     Data buffer
 *  @param numbytes Number of bytes in buffer to convert
 *
 *  @return 0 upon success
 */
int __stdcall nums(BYTE *buff, int numbytes);

/**
 *  @brief Convert data in buffer from numbers to characters
 *
 *  @param buff     Data buffer
 *  @param numbytes Number of bytes in buffer to convert
 *
 *  @return 0 upon success
 */
int __stdcall chars(BYTE *buff, int numbytes);

#ifdef __cplusplus
}
#endif

#endif  // #ifndef _CRYPT_H_

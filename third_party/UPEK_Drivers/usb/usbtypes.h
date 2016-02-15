/**
 * @file types.h
 *
 * This module defines types which if used ease 
 * porting effort significantly.
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */

#ifndef TYPES_H
#define TYPES_H

#include "types.h"

/*
nothing doing macros used for better documenting parameter specification
*/

/** 
 * This is a definition which has sole purpose of 
 * helping readability.  It indicates that formal
 * parameter is an input parameter.
 */
#ifndef IN
#define IN
#endif

/** 
 * This is a definition which has sole purpose of 
 * helping readability.  It indicates that formal
 * parameter is an output parameter.  
 */
#ifndef OUT
#define OUT
#endif

/** 
 * This is a definition which has sole purpose of 
 * helping readability.  It indicates that formal
 * parameter is both input and output parameter.  
 */
#ifndef INOUT 
#define INOUT
#endif

/** 
 * This is a definition which has sole purpose of 
 * helping readability.  It indicates that formal
 * parameter is an optional parameter.  
 */
#ifndef OPTIONAL
#define OPTIONAL
#endif

/*
 * ARC processor specific type mappings 
 */
 
/** Signed 8 bit integer */
typedef signed char    sint8;
/** Unsigned 8 bit integer */
typedef unsigned char  uint8;
/** Signed 16 bit integer */
typedef signed short   sint16;
/** Unsigned 16 bit integer */
typedef unsigned short uint16;
/** Signed 32 bit integer */
typedef signed long    sint32;
/** Unsigned 32 bit integer */
typedef unsigned long  uint32;
/** Single precision floating point number */
typedef float          float32;
/** Double precision floating point number */
typedef double         float64;

/**
 * Boolean false value
 */
#ifndef FALSE
#define FALSE           0
#endif /* #ifndef FALSE */

/**
 * Boolean true value
 */
#ifndef TRUE
#define TRUE            1
#endif /* #ifndef TRUE */

/**
 * NULL value
 */
#ifndef NULL
#define NULL            (0)
#endif /* #ifndef NULL */

/**
 * Generic function pointer type
 * with no return value and no
 * arguments.  
 */
typedef void (*v_v_func_ptr)(void);

/**
 * Callback function pointer type
 * which takes argument.
 */
typedef void (*callback_func_ptr)(void *);

/**
 * Void pointer is an architecture
 * independent type.
 */
#ifndef _WIN32
typedef void *PVOID;
#endif

// Macro for declaring pointers to unaligned data
#if defined(_ARC)
# define UNALIGNED _Unaligned
#else
# ifndef UNALIGNED
#  define UNALIGNED
# endif
#endif //#if defined(_ARC)

// ARC HC compiler does not declare any form of min or max macros, but it has
// intrinsic _min and _max functions.
#if defined(_ARC)
#define min(x,y) _min((x),(y))
#define max(x,y) _max((x),(y))
#endif

#ifndef UNREFERENCED_PARAMETER
// Macro must be empty for ARM compiler, otherwise we get lots of warnings
#define UNREFERENCED_PARAMETER(x)
#endif

// Macro for aligning value to next nearest DWORD boundary
#define DW_ALIGN(value) ((((uint32)(value) + 3) & 0xfffffffc))

// Macro for aligning uint8 pointer to next nearest DWORD boundary
#define PDW_ALIGN(pointer) ((uint8*)(((uint32)(pointer) + 3) & 0xfffffffc))


#endif

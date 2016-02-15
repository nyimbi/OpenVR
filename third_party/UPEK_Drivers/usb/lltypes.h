/**
 * @file lltypes.h
 *
 * communication structures and types
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */

#ifndef __LLTYPES_H__
#define __LLTYPES_H__
#include "usbtypes.h"


#define BUFFER_AS_UINT32(x) (*((uint32 *)x)) 

///Communiction type
typedef enum 
{
    LL_CONN_SERIAL	=1,
    LL_CONN_USB		=2,
    LL_CONN_REMOTE  =3, //session opened by PTOpenRemote()


} LL_CONN_TYPE,*PLL_CONN_TYPE;

typedef struct 
{
    char msDeviceName[1024];
    uint32 mdwPid;
    uint32 mdwVid;
    uint32 mdwDeviceType;
    uint32 mdwFlags;
} LL_USB,*PLL_USB;


typedef struct
{
    LL_CONN_TYPE mConectionType;
	LL_USB mUSB;
    uint32 mdwTimeout;
    
}LL_CONN_PARAMS,*PLL_CONN_PARAMS;

struct __LL_CONN_STRUCT__;
typedef struct __LL_CONN_STRUCT__ *LL_CONN_HANDLE,**PLL_CONN_HANDLE;



typedef struct __LL_CONN_STRUCT__
{
    LL_CONN_PARAMS 	mConnParams;
    void           *mhHandle;
    uint32		mdwDriverFlags;
} LL_CONN_STRUCT ;// *LL_CONN_HANDLE declared in forward declaration;	




#endif //#ifndef __LLTYPES_H__

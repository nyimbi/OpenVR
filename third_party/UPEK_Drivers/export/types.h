#ifndef _TYPES_H_
#define _TYPES_H_

#define FAR
#define TRUE  1
#define FALSE 0
#ifndef NULL
#define NULL  0
#endif
#define __stdcall

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

typedef unsigned char  BYTE;
typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned short WORD; 
typedef unsigned int   DWORD;
typedef unsigned int   UINT;
typedef unsigned int   ULONG;
typedef void           *HANDLE;
typedef int            BOOL;


#ifdef __cplusplus
extern "C" {
#endif

void Sleep(int ms);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _TYPES_H_

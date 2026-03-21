/////////////////////////////////////////////////////////////////////
// 
// (C) 2020 Panasonic Corporation
// 
// LMX_lib_type.h 
// 
// Lmxptpif.Definition necessary for using DLL 
// APL/DLL 
// 
// 
/////////////////////////////////////////////////////////////////////

#ifndef _LMX_LIB_TYPE_H__
#define _LMX_LIB_TYPE_H__

#include "lmx_lib_plat.h"

#ifdef LMX_PLAT_WINDOWS
#include <windows.h>
//#define INT8				char
//#define UINT8				unsigned char
//#define INT16				short
//#define UINT16				unsigned short
//#define INT32				long
//#define UINT32				unsigned long
//#define UTF16				unsigned short
#define LMX_BOOL			unsigned char
#endif // LMX_PLAT_WINDOWS

#ifdef LMX_PLAT_MACOS
#define INT8				char
#define UINT8				unsigned char
#define INT16				short
#define UINT16				unsigned short
#define INT32				int
#define UINT32				unsigned int
#define UINT64				unsigned long
#define UTF16				unsigned short
#define LMX_BOOL			unsigned char

#define VOID void
typedef wchar_t WCHAR;     
typedef char CHAR;
typedef unsigned char UCHAR;

typedef short SHORT;
typedef long LONG;
typedef int INT;

typedef unsigned long       DWORD;
typedef signed char         BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef FLOAT               *PFLOAT;
typedef BOOL            *PBOOL;
typedef BOOL             *LPBOOL;
typedef BYTE            *PBYTE;
typedef BYTE             *LPBYTE;
typedef int             *PINT;
typedef int              *LPINT;
typedef WORD            *PWORD;
typedef WORD             *LPWORD;
typedef long             *LPLONG;
typedef DWORD           *PDWORD;
typedef DWORD            *LPDWORD;
typedef void             *LPVOID;
//typedef CONST void       *LPCVOID;

typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int        *PUINT;


typedef void                *HANDLE;
typedef HANDLE              HGLOBAL;

typedef int                 HINSTANCE;
typedef int                 HMODULE;
typedef int                 HWND;


#define MAKELPARAM(h,l)     (((h & 0xffff) << 16) | (l & 0xffff))


// --------------
typedef unsigned long SIZE_T, *PSIZE_T;
typedef CHAR *NPSTR, *LPSTR, *PSTR;
typedef WCHAR *NPWSTR, *LPWSTR, *PWSTR;
typedef const char* LPCSTR;

#ifndef NULL
  #ifdef __cplusplus
    #define NULL 0
  #else
    #define NULL ((void *)0)
  #endif
#endif

#define MAX_PATH 260

#endif // LMX_PLAT_MACOS

#define LMX_BOOL_TRUE		1
#define LMX_BOOL_FALSE		0


/////////////////////////////////////////////////////////////////////
//
// Type declaration definition 
//
/////////////////////////////////////////////////////////////////////
#define LMX_DEF_USER_PTP_ARRAY_MAX	512		// 

#define LMX_DEF_USER_PTP_STRING_MAX	256		// 



/////////////////////////////////////////////////////////////////////
// 
// String Format 
//
/////////////////////////////////////////////////////////////////////
typedef struct _tag_LMX_STRUCT_PTP_ARRAY_STRING{
	UINT8 NumChars;													// 
	UINT8 StringChars[LMX_DEF_USER_PTP_STRING_MAX];					// Unicode null-terminated String 
	UINT8 Available;												// bit0: ARRAY Valid (1) Invalid (0)
}LMX_STRUCT_PTP_ARRAY_STRING;


//--- FORM ENUM : UINT16 ---//
typedef struct _tag_LMX_STRUCT_PTP_FORM_ENUM_UINT16{				// FORM ENUM structure:UINT16
	UINT16 NumOfVal;												// Element count
	UINT16 SupportVal[LMX_DEF_USER_PTP_ARRAY_MAX];					// Element value
	UINT8  Available;												// bit0: ENUM valid (1) invalid (0), bit 1: CurVal enabled (1) invalid (0)
}LMX_STRUCT_PTP_FORM_ENUM_UINT16;


//--- FORM ENUM : UINT32 ---//
typedef struct _tag_LMX_STRUCT_PTP_FORM_ENUM_UINT32{				// FORM ENUM structure:UINT32
	UINT16 NumOfVal;												// Element count
	UINT32 SupportVal[LMX_DEF_USER_PTP_ARRAY_MAX];					// Element value
	UINT8  Available;												// bit0: ENUM valid (1) invalid (0), bit 1: CurVal enabled (1) invalid (0)
}LMX_STRUCT_PTP_FORM_ENUM_UINT32;


typedef struct _tag_LMX_STRUCT_PTP_FORM_RANGE_UINT16{				// FORM RANGE structure:UINT16
	UINT16	MinVal;													// Minimum value
	UINT16	MaxVal;													// Maximum value
	UINT16	StepSize;												// Number of steps
	UINT8   Available;												// bit0: RANGE Valid (1) Invalid (0)
}LMX_STRUCT_PTP_FORM_RANGE_UINT16;


//--- FORM RANGE : INT32 ---//
typedef struct _tag_LMX_STRUCT_PTP_FORM_RANGE_UINT32{				// FORM RANGE structure:UINT32
	UINT32	MinVal;													// Minimum value
	UINT32	MaxVal;													// Maximum value
	UINT32	StepSize;												// Number of steps
	UINT8   Available;												// bit0: RANGE Valid (1) Invalid (0)
}LMX_STRUCT_PTP_FORM_RANGE_UINT32;


#endif // _LMX_LIB_TYPE_H__


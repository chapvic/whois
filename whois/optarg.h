/***************************************************************************

MIT License

Copyright (c) 2021 FoxTeam

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*****************************************************************************/

#ifndef OPTARG_H
#define OPTARG_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "memory.h"

#ifndef ARRAY_SIZE
#ifdef ARRAYSIZE
#define ARRAY_SIZE(A) ARRAYSIZE(A)
#else
#define ARRAY_SIZE(A) (sizeof(A)/sizeof((A)[0]))
#endif // ARRAYSIZE
#endif // !ARRAY_SIZE

#define E_OPTARG_SUCCESS           0    // Success, no errors
#define E_OPTARG_INVALID_OPTION   -1    // Invalid option
#define E_OPTARG_NO_PARAM         -2    // Parameter not found
#define E_OPTARG_PARAM_IS_OPTION  -3    // Parameter is option
#define E_OPTARG_AFTER_NON_OPTION -4    // Option after non-option value
#define E_OPTARG_ALREADY_EXIST    -5    // Option already exist (duplicate)

#pragma pack(push, _CRT_PACKING)

typedef struct _OPTARG {
	// initial values
	TCHAR* name;               // short option name
	TCHAR* longname;           // long option name
	TCHAR* param;              // option parameter description
	TCHAR* help;               // option usage information (help string)
	// return values
	BOOL flag;                 // TRUE, if option need a param
	BOOL exist;                // TRUE, if option found
	BOOL longopt;              // TRUE, if option has long name
	TCHAR* value;              // param value, if exist
} OPTARG, *POPTARG;

typedef struct _OPTARG_LIST {
	size_t size;                // list size
	TCHAR** items;              // array of items
} OPTARG_LIST, *POPTARG_LIST;

typedef struct _OPTARG_RESULT {
	int error;
	TCHAR* invalid;
	POPTARG optarg;
	POPTARG_LIST list;
} OPTARG_RESULT, *POPTARG_RESULT;

#pragma pack(pop)

// ------------------------------------------------------
//
// WARNING !!!
// These variables must be initialized in the source code
//
extern OPTARG __optarg[]; // list of options
extern size_t __optarg_size;   // size of list
//
// ------------------------------------------------------

// Internal optarg result structure
extern OPTARG_RESULT __optarg_result;

void __cdecl optarg_init(int argc, TCHAR** argv);
void __cdecl optarg_free(void);
void __cdecl optarg_help(void);

#endif // !OPTARG_H

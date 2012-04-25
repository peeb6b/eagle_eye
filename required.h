/*
    This file is part of SeeBorg.
	Copyright (C) 2003, 2006 Eugene Bujak.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

*/

#ifndef __SEEBORG_REQUIRED_H__
#define __SEEBORG_REQUIRED_H__

#ifdef _MSC_VER
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)
#pragma warning (disable: 4786)
#pragma warning (disable: 4100)
#pragma warning (disable: 4702)
#pragma warning (disable: 4710)
#pragma warning (disable: 4127)
#define inline __inline
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>

#ifdef _WIN32
#include <windows.h>
#define strcasecmp stricmp
#define wcscasecmp wcsicmp
#define wcsncasecmp wcsnicmp
#define snprintf _snprintf
#define snwprintf _snwprintf
#define vsnwprintf _vsnwprintf
#define wfopen _wfopen
#define wtoi _wtoi
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#define vsnwprintf vswprintf
#define snwprintf swprintf
#include <ctype.h>
#include <inttypes.h>
#endif

#if defined(__GNUC__) && (__GNUC__ >= 3)
# define likely(x)    (__builtin_expect((x), 1))
# define unlikely(x)  (__builtin_expect((x), 0))
#else
# define likely(x) (x)
# define unlikely(x) (x)
#endif

#if defined __GNUC__ && __GNUC__ >= 4 && !defined _WIN32 && !defined __CYGWIN__
# define VIS_PUBLIC __attribute__ ((visibility("default")))
# define VIS_LOCAL  __attribute__ ((visibility("hidden")))
#elif (defined _WIN32 || defined __CYGWIN__)
# ifdef BUILDING_DLL
#  define VIS_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
# else
#  define VIS_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
# endif
# define VIS_LOCAL
#else
# define VIS_PUBLIC
# define VIS_LOCAL
#endif

#if defined __GNUC__ && __GNUC__ >= 2 && __GNUC_MINOR__ >= 7
#  define UNUSED(var) var __attribute__((unused))
#else
#  if defined(__cplusplus)
#    define UNUSED(var)
#  else
#    define UNUSED(var) var
#  endif
#endif


#define SEEBORGVERSIONMAJOR 0
#define SEEBORGVERSIONMINOR 10
#define SEEBORGVERSIONWSTRING L"0.10 beta"
#define SEEBORGVERSIONSTRING "0.10 beta"

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#undef safe_free
#define safe_free(ptr) {if (ptr) free(ptr); ptr = NULL;}

static inline wchar_t* wva(const wchar_t* format, ...) {
	static wchar_t str[4][16384];
	static int	index = 0;

	wchar_t* buf = str[index++ & 3];

	va_list argptr;
	va_start (argptr, format);
	vsnwprintf (buf, 16384, format, argptr);
	va_end(argptr);

#ifdef _MSC_VER
	buf[16383] = 0;
#endif

	return buf;
}

// Use to print wide-character string into console, only stdout and stderr are
// accepted. If you provide anything else besides that, it'll default to stdout.
static inline void see_printstring(FILE *f, const wchar_t* format, ...) {
#ifdef _WIN32
  HANDLE outhandle;
  DWORD numwritten = 0;
#endif
  wchar_t outbuf[16384];
  int retval;

  va_list argptr;
  va_start (argptr, format);
  retval = vsnwprintf (outbuf, 16384, format, argptr);
  va_end(argptr);
#ifdef _WIN32
  outbuf[16383] = L'\0';
  if (f == stderr) outhandle = GetStdHandle(STD_ERROR_HANDLE);
  else outhandle = GetStdHandle(STD_OUTPUT_HANDLE);
  WriteConsoleW(outhandle, outbuf, retval, &numwritten, NULL);
#else
  if (f != stderr) f = stdout;
  fputws(outbuf, f);
#endif
}

#ifdef __cplusplus
}
using namespace std;
#endif

#endif // __SEEBORG_REQUIRED_H__

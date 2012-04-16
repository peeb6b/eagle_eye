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

#include "required.h"

#ifndef __SEEUTIL_H__
#define __SEEUTIL_H__

#include <string>
#include <vector>

#include "utf8.h"



// String functions

// splitString - split a string from str to tokens, using needle.
//               by default, needle is whitespace.
//               if tokens argument contains data, new data will be appended.
size_t splitString(IN const wstring &str, OUT vector<wstring> &tokens, IN const wchar_t* needle = L" ");
OUT wstring joinString(IN vector<wstring> &tokens, wstring separator = L" ");
int lowerString(IN OUT wstring &str);
int lowerString(IN OUT wchar_t *str);
int trimString(IN OUT wstring &str, IN bool punct = false);
int FilterMessage(IN OUT wstring &message);

// file/string functions
int fReadStringLine(IN FILE *f, OUT wstring &outstr);

// Other helpful functions
inline int randInt(const int min, const int max) {
  if (max < min) return min;
  return (rand() % (max - min + 1)) + min;
}

inline float randFloat(const float min, const float max) {
  if (max < min) return min;
  float retval = (float)rand()/(float)RAND_MAX;
  retval *= (max - min);
  retval += min;
  return retval;
}

// argument-style tokenizer
typedef struct tokenizer_s {
	size_t argc;
	wchar_t **argv;
} tokenizer_t;
static const wchar_t *tokenizer_nullstring = L"";
static const size_t tokenizer_sizestep = 256;

// always call this first
tokenizer_t*   tokenizer_init(void);
// all tokens become empty if you pass NULL string into it
size_t         tokenizer_tokenize(tokenizer_t* tokenizer, IN const wchar_t* wcstr);
size_t         tokenizer_argc(tokenizer_t* tokenizer);
const wchar_t* tokenizer_argv(tokenizer_t* tokenizer, IN const size_t index);
void           tokenizer_free(tokenizer_t* tokenizer);


// utf8/wstring handling routines
bool utf8_mbstowstring(IN const char *mbstr, OUT wstring &wstr);
// Don't forget to safe_free() the return value
OUT char* utf8_wstringtombs(IN const wstring &wstr);
#endif

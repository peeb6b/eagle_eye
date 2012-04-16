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

#include <stdio.h>

#include "required.h"
#include "utf8.h"

/*
** UTF-8 equivalent of the C library's mbtowc().
** Taken from Xiph.org Ogg Vorbis project.
**
** Return value less than zero means an error.
** Don't use MB_CUR_MAX for n, size that macro uses current locale -
** use 6 as maximum instead.
*/
int utf8_mbtowc(wint_t *pwc, const char *s, size_t n)
{
  unsigned char c;
  int wc;
  size_t k, i;
  
  if (!n || !s)
    return 0;
  
  c = *s;
  if (c < 0x80) {
    if (pwc)
      *pwc = c;
    return c ? 1 : 0;
  }
  else if (c < 0xc2)
    return -1;
  else if (c < 0xe0) {
    if (n >= 2 && (s[1] & 0xc0) == 0x80) {
      if (pwc)
		*pwc = ((c & 0x1f) << 6) | (s[1] & 0x3f);
      return 2;
    }
    else
      return -1;
  }
  else if (c < 0xf0)
    k = 3;
  else if (c < 0xf8)
    k = 4;
  else if (c < 0xfc)
    k = 5;
  else if (c < 0xfe)
    k = 6;
  else
    return -1;
  
  if (n < k)
    return -1;
  wc = *s++ & ((1 << (7 - k)) - 1);
  for (i = 1; i < k; i++) {
    if ((*s & 0xc0) != 0x80)
      return -1;
    wc = (wc << 6) | (*s++ & 0x3f);
  }
  if (wc < (1 << (5 * k - 4)))
    return -1;
  if (pwc)
    *pwc = wc;
  return k;
}

/*
** UTF-8 equivalent of the C library's wctomb().
** Taken from Xiph.org Ogg Vorbis project. With little modifications.
**
** Return value less than zero means an error.
*/
int utf8_wctomb(char *s, wint_t wc1)
{
  unsigned int wc = wc1;
  
  if (!s)
    return 0;
  if (wc < (1 << 7)) {
    *s++ = (char)wc;
    return 1;
  }
  else if (wc < (1 << 11)) {
    *s++ = 0xc0 | (wc >> 6);
    *s++ = 0x80 | (wc & 0x3f);
    return 2;
  }
  else if (wc < (1 << 16)) {
    *s++ = 0xe0 | (wc >> 12);
    *s++ = 0x80 | ((wc >> 6) & 0x3f);
    *s++ = 0x80 | (wc & 0x3f);
    return 3;
  }
  else if (wc < (1 << 21)) {
    *s++ = 0xf0 | (wc >> 18);
    *s++ = 0x80 | ((wc >> 12) & 0x3f);
    *s++ = 0x80 | ((wc >> 6) & 0x3f);
    *s++ = 0x80 | (wc & 0x3f);
    return 4;
  }
  else if (wc < (1 << 26)) {
    *s++ = 0xf8 | (wc >> 24);
    *s++ = 0x80 | ((wc >> 18) & 0x3f);
    *s++ = 0x80 | ((wc >> 12) & 0x3f);
    *s++ = 0x80 | ((wc >> 6) & 0x3f);
    *s++ = 0x80 | (wc & 0x3f);
    return 5;
  }
  else if (wc < (1 << 31)) {
    *s++ = 0xfc | (wc >> 30);
    *s++ = 0x80 | ((wc >> 24) & 0x3f);
    *s++ = 0x80 | ((wc >> 18) & 0x3f);
    *s++ = 0x80 | ((wc >> 12) & 0x3f);
    *s++ = 0x80 | ((wc >> 6) & 0x3f);
    *s++ = 0x80 | (wc & 0x3f);
    return 6;
  }
  else
    return -1;
}


/*
** UTF-8 equivalent of the C library's mbstowcs().
*/
size_t utf8_mbstowcs(wchar_t *wcstr, const char *mbstr, size_t count) {
  unsigned int i;
  unsigned int mbsize = 0;
  unsigned int wclen  = 0;
  int retval = 0;
  
  if (mbstr == NULL) return 0;
  
  if (wcstr == NULL) {
	// return the required size of the destination string
	for (i = 0;; i++) {
	  if (*(mbstr + mbsize) == 0) break;
	  retval = utf8_mbtowc(NULL, mbstr + mbsize, 6);
	  if (retval == 0) break;
	  if (retval < 0) return (size_t)-1;
	  
	  wclen++; mbsize += retval;
	}
	return wclen;
  }
  
  
  for (i = 0; i < count; i++) {
	if (*(mbstr + mbsize) == 0) {
	  wcstr[i] = L'\0';
	  break;
	}
	
	retval = utf8_mbtowc((int*)(wcstr + i), mbstr+mbsize, 6);
	if (retval == 0) break;
	if (retval < 0) return (size_t)-1;
	
	wclen++; mbsize += retval;
  }
  
  return wclen;
}

/*
** UTF-8 equivalent of the C library's wcstombs().
*/
size_t utf8_wcstombs(char *mbstr, const wchar_t *wcstr, size_t count) {
  size_t i, j, mbsize = 0;
  int retval = 0;
  char tempmb[6];
  
  if (wcstr == NULL) return 0;
  
  if (mbstr == NULL) {
	// return the required size of the destination string
	for (i = 0;; i++) {
	  if (wcstr[i] == L'\0') break;
	  retval = utf8_wctomb(tempmb, wcstr[i]);
	  if (retval == 0) break;
	  if (retval < 0) return (size_t)-1;
	  mbsize += retval;
	}
	return mbsize;
  }
  
  for (i = 0;; i++) {
	if (wcstr[i] == L'\0') {
	  if (mbsize+1 <= count) *(mbstr+mbsize) = 0;
	  break;
	}
	retval = utf8_wctomb(tempmb, wcstr[i]);
	if (retval == 0) break;
	if (retval < 0) return (size_t)-1;
	
	if (mbsize+retval <= count) {
	  // There's enough room for another mbchar
	  for (j = 0; j < retval; j++) *(mbstr+mbsize+j) = tempmb[j];
	  mbsize += retval;
	} else break;
  }
  
  return mbsize;
}


/*
** =============================================================
** UTF-8 writer (writes wide-character string as UTF8 into file)
** =============================================================
*/
utf8writer_t* utf8writer_init(void) {
  utf8writer_t* utf8writer = malloc(sizeof(utf8writer_t));
  if (utf8writer == NULL) return NULL;
  
  utf8writer->utf8strsize = utf8writer_defsize;
  utf8writer->utf8str = malloc(utf8writer_defsize);
  if (utf8writer->utf8str == NULL) {
	safe_free (utf8writer);
	return NULL;
  }
  
  return utf8writer;
}

int utf8writer_write(IN OUT utf8writer_t *utf8writer, IN FILE* f, 
					 IN const wchar_t* wcstr, IN const char* format) 
{
  size_t retval;
  
  if (utf8writer == NULL) return 0;
  if (utf8writer->utf8str == NULL) return 0;
  if (f == NULL) return 0;
  
  retval = utf8_wcstombs(NULL, wcstr, 0);
  if (retval+1 > utf8writer->utf8strsize) {
	utf8writer->utf8str = (char*)realloc(utf8writer->utf8str, retval+1);
	if (utf8writer->utf8str == NULL) return 0;
	utf8writer->utf8strsize = retval+1;
  }
  
  retval = utf8_wcstombs(utf8writer->utf8str, wcstr, retval+1);
  
  if (format == NULL) fprintf (f, "%s", utf8writer->utf8str);
  else fprintf (f, format, utf8writer->utf8str);
  
  return 1;
}

void utf8writer_free(utf8writer_t* utf8writer) {
  if (utf8writer == NULL) return;
  
  if (utf8writer->utf8str != NULL) free(utf8writer->utf8str);
  safe_free (utf8writer);
  
  return;
}

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

#ifndef __SEEBORG_UTF8_H__
#define __SEEBORG_UTF8_H__

#include "required.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * UTF-8 equivalent of the C library's mbtowc().
 * Taken from Xiph.org Ogg Vorbis project.
 *
 * Return value less than zero means an error.
 * Don't use MB_CUR_MAX for n, size that macro uses current locale -
 * use 6 as maximum instead.
 */
int utf8_mbtowc(wint_t *pwc, const char *s, size_t n);

/*
 * UTF-8 equivalent of the C library's wctomb().
 * Taken from Xiph.org Ogg Vorbis project.
 *
 * Return value less than zero means an error.
 */
int utf8_wctomb(char *s, wint_t wc1);

/*
 * UTF-8 equivalent of the C library's mbstowcs().
 *
 * Recommended usage:
 * retval = utf8_mbstowcs(NULL, mbstr, 0);
 * wcstr = (wchar_t*)malloc((retval+1)*sizeof(wchar_t));
 * retval = utf8_mbstowcs(wcstr, mbstr, strlen(mbstr) + 1);
 * fwrite(wcstr, 1, (retval+1)*sizeof(wchar_t), f);
 * free(wcstr);
 */
size_t utf8_mbstowcs(wchar_t *wcstr, const char *mbstr, size_t count);

/*
 * UTF-8 equivalent of the C library's wcstombs().
 *
 * Recommended usage:
 * retval = utf8_wcstombs(NULL, wcstr, 0);
 * mbstr = (char*)malloc(retval+1);
 * retval = utf8_wcstombs(mbstr, wcstr, retval+1);
 * fwrite(mbstr, 1, retval+1, f);
 * free(mbstr);
 */
size_t utf8_wcstombs(char *mbstr, const wchar_t *wcstr, size_t count);


/*
 * =============================================================
 * UTF-8 writer (writes wide-character string as UTF8 into file)
 * =============================================================
 */
const size_t utf8writer_defsize = 65536;
typedef struct utf8writer_s {
	size_t utf8strsize;
	char* utf8str;
} utf8writer_t;

utf8writer_t* utf8writer_init(void);
int utf8writer_write(IN OUT utf8writer_t *utf8writer, IN FILE* f, 
					 IN const wchar_t* wcstr, IN const char* format);
void utf8writer_free(utf8writer_t *utf8writer);

#ifdef __cplusplus
}
#endif

#endif // __SEEBORG_UTF8_H__

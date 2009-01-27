/* music player command (mpc)
 * Copyright (C) 2003-2008 Warren Dukes <warren.dukes@gmail.com>,
				Eric Wong <normalperson@yhbt.net>,
				Daniel Brown <danb@cs.utexas.edu>
 * Copyright (C) 2008-2009 Max Kellermann <max@duempel.org>
 * Project homepage: http://musicpd.org
 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "charset.h"

#include "mpc.h"
#include "gcc.h"

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <locale.h>
#include <langinfo.h>
#include <iconv.h>

static bool charset_enable_input;
static bool charset_enable_output;
static char *locale_charset;

static iconv_t char_conv_iconv;
static char * char_conv_to;
static char * char_conv_from;
static int ignore_invalid;

#define BUFFER_SIZE	1024

static void
charset_close(void);

/* code from iconv_prog.c (omiting invalid symbols): */
static inline char * mpc_strchrnul(const char *s, int c)
{
	char *ret = strchr(s, c);
	if (!ret)
		ret = strchr(s, '\0');
	return ret;
}

static char * skip_invalid(const char *to)
{
	const char *errhand = mpc_strchrnul(to, '/');
	int nslash = 2;
	char *newp, *cp;

	if (*errhand == '/') {
		--nslash;
		errhand = mpc_strchrnul(errhand, '/');

		if (*errhand == '/') {
			--nslash;
			errhand = strchr(errhand, '\0');
		}
	}

	newp = (char *)malloc(errhand - to + nslash + 7 + 1);
	memcpy(newp, to, errhand - to);
	cp = newp + (errhand - to);
	while (nslash-- > 0)
		*cp++ = '/';
	if (cp[-1] != '/')
		*cp++ = ',';
	memcpy(cp, "IGNORE", sizeof("IGNORE"));
	return newp;
}

static int
charset_set2(const char *to, const char *from)
{
	if(char_conv_to && strcmp(to,char_conv_to)==0 &&
			char_conv_from && strcmp(from,char_conv_from)==0)
		return 0;

	charset_close();

	if ((char_conv_iconv = iconv_open(to,from))==(iconv_t)(-1))
		return -1;

	char_conv_to = strdup(to);
	char_conv_from = strdup(from);
	return 0;
}

static int
charset_set(const char *to, const char *from)
{
	char *allocated;
	int ret;

	if (ignore_invalid)
		to = allocated = skip_invalid(to);
	else
		allocated = NULL;

	ret = charset_set2(to, from);

	if (allocated != NULL)
		free(allocated);

	return ret;
}

static inline size_t deconst_iconv(iconv_t cd,
				   const char **inbuf, size_t *inbytesleft,
				   char **outbuf, size_t *outbytesleft)
{
	union {
		const char **a;
		char **b;
	} deconst;

	deconst.a = inbuf;

	return iconv(cd, deconst.b, inbytesleft, outbuf, outbytesleft);
}

static char *
charset_conv_strdup(mpd_unused const char *string)
{
	char buffer[BUFFER_SIZE];
	size_t inleft = strlen(string);
	char * ret;
	size_t outleft;
	size_t retlen = 0;
	size_t err;
	char * bufferPtr;

	if(!char_conv_to) return NULL;

	ret = strdup("");

	while(inleft) {
		bufferPtr = buffer;
		outleft = BUFFER_SIZE;
		err = deconst_iconv(char_conv_iconv,&string,&inleft,&bufferPtr,
				    &outleft);
		if (outleft == BUFFER_SIZE ||
		    (err == (size_t)-1 && errno != E2BIG)) {
			free(ret);
			return NULL;
		}

		ret = realloc(ret,retlen+BUFFER_SIZE-outleft+1);
		memcpy(ret+retlen,buffer,BUFFER_SIZE-outleft);
		retlen+=BUFFER_SIZE-outleft;
		ret[retlen] = '\0';
	}

	return ret;
}

static void
charset_close(void)
{
	if(char_conv_to) {
		iconv_close(char_conv_iconv);
		free(char_conv_to);
		free(char_conv_from);
		char_conv_to = NULL;
		char_conv_from = NULL;
	}
}

void
charset_init(bool enable_input, bool enable_output)
{
	const char *original_locale, *charset;

	if (!enable_input && !enable_output)
		return;

	ignore_invalid = isatty(STDOUT_FILENO) && isatty(STDIN_FILENO);

	original_locale = setlocale(LC_CTYPE,"");
	if (original_locale == NULL)
		return;

	charset = nl_langinfo(CODESET);
	if (charset != NULL)
		locale_charset = strdup(charset);

	setlocale(LC_CTYPE,original_locale);

	if (locale_charset != NULL) {
		charset_enable_input = enable_input;
		charset_enable_output = enable_output;
	}
}

void charset_deinit(void)
{
	charset_close();

	free(locale_charset);
}

const char *
charset_to_utf8(const char *from) {
	static char * to = NULL;

	if (!charset_enable_input)
		/* no locale: return raw input */
		return from;

	if(to) free(to);

	charset_set("UTF-8", locale_charset);
	to = charset_conv_strdup(from);

	if (to == NULL)
		return from;

	return to;
}

const char *
charset_from_utf8(const char *from) {
	static char * to = NULL;

	if (!charset_enable_output)
		/* no locale: return raw UTF-8 */
		return from;

	if(to) free(to);

	charset_set(locale_charset, "UTF-8");
	to = charset_conv_strdup(from);

	if (to == NULL)
		return from;

	return to;
}

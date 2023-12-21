// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "charset.h"

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

/* code from iconv_prog.c (omitting invalid symbols): */
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

	if (*errhand == '/') {
		--nslash;
		errhand = mpc_strchrnul(errhand, '/');

		if (*errhand == '/') {
			--nslash;
			errhand = strchr(errhand, '\0');
		}
	}

	char *newp = (char *)malloc(errhand - to + nslash + 7 + 1);
	memcpy(newp, to, errhand - to);
	char *cp = newp + (errhand - to);
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
	if (ignore_invalid)
		to = allocated = skip_invalid(to);
	else
		allocated = NULL;

	int ret = charset_set2(to, from);

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
charset_conv_strdup(const char *string)
{
	if(!char_conv_to) return NULL;

	size_t inleft = strlen(string);
	size_t retlen = 0;

	char *ret = strdup("");

	while(inleft) {
		char buffer[BUFFER_SIZE];
		char *bufferPtr = buffer;
		size_t outleft = BUFFER_SIZE;
		size_t err = deconst_iconv(char_conv_iconv,
					   &string, &inleft, &bufferPtr,
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
	if (!enable_input && !enable_output)
		return;

	ignore_invalid = isatty(STDOUT_FILENO) && isatty(STDIN_FILENO);

	const char *original_locale = setlocale(LC_CTYPE,"");
	if (original_locale == NULL)
		return;

	const char *charset = nl_langinfo(CODESET);
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

	free(to);

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

	free(to);

	charset_set(locale_charset, "UTF-8");
	to = charset_conv_strdup(from);

	if (to == NULL)
		return from;

	return to;
}

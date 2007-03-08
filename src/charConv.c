/* the Music Player Daemon (MPD)
 * (c)2003-2004 by Warren Dukes (warren.dukes@gmail.com)
 * This project's homepage is: http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "charConv.h"

#include "mpc.h"

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_LOCALE
#ifdef HAVE_LANGINFO_CODESET
#include <locale.h>
#include <langinfo.h>
#endif
#endif

static char * localeCharset;

#ifdef HAVE_ICONV
#include <iconv.h>
static iconv_t char_conv_iconv;
static char * char_conv_to;
static char * char_conv_from;
static int ignore_invalid;
#endif

#define BUFFER_SIZE	1024
#ifdef HAVE_ICONV
static void closeCharSetConversion();
#endif

/* code from iconv_prog.c (omiting invalid symbols): */
#ifdef HAVE_ICONV
static char * skip_invalid(char *to)
{
	const char *errhand = strchrnul(to, '/');
	int nslash = 2;
	char *newp, *cp;

	if (*errhand == '/') {
		--nslash;
		errhand = strchrnul (errhand, '/');

		if (*errhand == '/') {
			--nslash;
			errhand = strchr(errhand, '\0');
		}
	}

	newp = (char *)malloc(errhand - to + nslash + 7 + 1);
	cp = mempcpy(newp, to, errhand - to);
	while (nslash-- > 0)
		*cp++ = '/';
	if (cp[-1] != '/')
		*cp++ = ',';
	memcpy(cp, "IGNORE", sizeof("IGNORE"));
	return newp;
}
#endif

static int setCharSetConversion(char * to, char * from) {
#ifdef HAVE_ICONV
	if (ignore_invalid)
		to = skip_invalid(to);
	if(char_conv_to && strcmp(to,char_conv_to)==0 &&
			char_conv_from && strcmp(from,char_conv_from)==0)
		return 0;

	closeCharSetConversion();

	if ((char_conv_iconv = iconv_open(to,from))==(iconv_t)(-1))
		return -1;

	char_conv_to = strdup(to);
	char_conv_from = strdup(from);

	if (ignore_invalid)
		free(to);

	return 0;
#endif
	return -1;
}

static char * convStrDup(char * string) {
#ifdef HAVE_ICONV
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
		err = iconv(char_conv_iconv,&string,&inleft,&bufferPtr,
					&outleft);
		if(outleft==BUFFER_SIZE || (err<0 && errno!=E2BIG)) {
			free(ret);
			return NULL;
		}

		ret = realloc(ret,retlen+BUFFER_SIZE-outleft+1);
		memcpy(ret+retlen,buffer,BUFFER_SIZE-outleft);
		retlen+=BUFFER_SIZE-outleft;
		ret[retlen] = '\0';
	}

	return ret;
#endif
	return NULL;
}

#ifdef HAVE_ICONV
static void closeCharSetConversion(void) {
	if(char_conv_to) {
		iconv_close(char_conv_iconv);
		free(char_conv_to);
		free(char_conv_from);
		char_conv_to = NULL;
		char_conv_from = NULL;
	}
}
#endif

void setLocaleCharset(void) {
#ifdef HAVE_LOCALE
#ifdef HAVE_LANGINFO_CODESET
        char * originalLocale;
        char * charset = NULL;

#ifdef HAVE_ICONV
	ignore_invalid = isatty(STDOUT_FILENO) && isatty(STDIN_FILENO);
#endif

	if((originalLocale = setlocale(LC_CTYPE,""))) {
                char * temp;
                                                                                
                if((temp = nl_langinfo(CODESET))) {
                        charset = strdup(temp);
                }
                if(!setlocale(LC_CTYPE,originalLocale));
        }
	
	if(localeCharset) free(localeCharset);
                                                                                
        if(charset) {
		localeCharset = strdup(charset);
                free(charset);
		return;
        }
#endif
#endif
        localeCharset = strdup("ISO-8859-1");
}

char * toUtf8(char * from) {
	static char * to = NULL;

	if(to) free(to);

	setCharSetConversion("UTF-8", localeCharset);
	to = convStrDup(from);

	if(!to) to = strdup(from);

	return to;
}

char * fromUtf8(char * from) {
	static char * to = NULL;

	if(to) free(to);

	setCharSetConversion(localeCharset, "UTF-8");
	to = convStrDup(from);

	if(!to) {
		/*printf("not able to convert: %s\n",from);*/
		to = strdup(from);
	}

	return to;
}

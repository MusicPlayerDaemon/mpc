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

#include "password.h"
#include "libmpdclient.h"
#include "util.h"
#include "mpc.h"

#include <string.h>
#include <stdlib.h>

void parse_password ( const char * host,
		int * password_len,
		int * parsed_len )
{
	/* parse password and host */
	char * ret = strstr(host,"@");
	int len = ret-host;

	if(ret && len == 0) parsed_len++;
	else if(ret) {
		* password_len = len;
		* parsed_len += len+1;
	}
}

void send_password ( const char * host,
		int password_len,
		mpd_Connection * conn)
{
	char * dup = strdup(host);
	dup[password_len] = '\0';

	mpd_sendPasswordCommand(conn,dup);
	printErrorAndExit(conn);
	mpd_finishCommand(conn);
	printErrorAndExit(conn);

	free(dup);
}


/* mpc
 * (c) 2003-2004 by normalperson and Warren Dukes (warren.dukes@gmail.com)
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

#ifndef MPC_H
#define MPC_H

#include "../config.h"

#include "libmpdclient.h"
#define STRING_LENGTH	(2*MAXPATHLEN)

#define STDIN_SYMBOL	"-"

#ifndef DEFAULT_HOST
#  define DEFAULT_HOST "localhost"
#endif /* DEFAULT_HOST */

#ifndef DEFAULT_PORT
#  define DEFAULT_PORT "6600"
#endif /* DEFAULT_PORT */

#ifndef MIN_COLUMNS
#  define MIN_COLUMNS	80
#endif /* MIN_COLUMNS */

typedef int (* cmdhandler)(int argc, char ** argv, mpd_Connection * conn);

#ifdef DEBUG
#  define dbg(fmt, arg...) do { printf("%s: "fmt,__func__, ## arg); } while (0)
#else
#  define dbg(fmt, arg...) do { } while (0)
#endif /* DEBUG */

#endif /* MPC_H */

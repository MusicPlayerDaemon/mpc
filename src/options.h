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

#ifndef OPTIONS_H
#define OPTIONS_H

struct mpc_option
{
	const char *name;   /* long name of the option */
	int has_value;      /* whether a value should follow this option */
	int set;            /* whether this option is on/off */
	char *value;        /* the value of the option (null if ! has_value) */
};

/* i like this type/instance naming scheme better than _mpc_table/mpc_table
    - danb */
extern struct mpc_option mpc_options [];

struct mpc_option *get_option(const char *option);
int parse_options (int * argc_p, char ** argv);

#endif /* OPTIONS_H */	

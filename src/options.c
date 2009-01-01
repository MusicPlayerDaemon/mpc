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

#include "options.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* i like this type/instance naming scheme better than _mpc_table/mpc_table
    - danb */
struct mpc_option mpc_options [] =
{
	{ "format", 1, 0, NULL },
	{ "no-status", 0, 0, NULL },

	/* other ideas for options...
	{ "host", 1, 0, NULL },
	{ "port", 1, 0, NULL },
	{ "verbose", 0, 0, NULL },
	*/

	{ .name = NULL }
};

/* why don't all languages have dictionary built-ins...? */
struct mpc_option *get_option(const char *option)
{
	int i;

	for (i = 0; mpc_options[i].name; ++i)
		if (strcmp(option, mpc_options[i].name) == 0)
			return &mpc_options[i];

	return NULL;
}

/* removes the index-th element from arr (*size gets decremented) */
static void remove_index (int idx, char ** arr, int * size)
{
	int i;

	/* these would be dumb... */
	assert(arr);
	assert(size);
	assert(idx >= 0);
	assert(*size >= 0);
	assert(idx < *size);

	/* shift everything past index one to the left and decrement the size */
	for (i = idx; i < *size - 1; ++i)
		arr[i] = arr[i + 1];
	--*size;
}

/* check and extract options from argv (between argv[0] and the first command
   argument, since other locations could be ambiguous) */
int parse_options (int * argc_p, char ** argv)
{
	int i;
	struct mpc_option * option;

	for (i = 1; i < *argc_p; )
	{
		/* args with a "--" prefix are options, until we find the "--" option */
		if (strncmp(argv[i], "--", 2) == 0)
		{
			/* quit parsing on the first "--" */
			if (strcmp(argv[i], "--") == 0)
				return 0;

			/* strip the prefix from our option and try to look it up */
			option = get_option(argv[i] + 2);
			if (option)
			{
				option->set = 1;

				/* do we need to grab a parameter for this option? */
				if (option->has_value)
				{
					/* are there any arguments left? */
					if (i < *argc_p - 1)
					{
						/* grab and extract the value */
						option->value = argv[i + 1];
						remove_index(i + 1, argv, argc_p);
					}
					else
					{
						fprintf(stderr, "Option %s expects a value\n",
								argv[i]);
						return -1;
					}
				}
			}
			else
			{
				fprintf(stderr, "Invalid option: %s\n", argv[i]);
				return -1;
			}

			/* remove the i-th element from argv */
			remove_index(i, argv, argc_p);
		}
		else
		{
			/* increment iff we don't remove an element of argv */
			++i;
		}
	}

	return 0;
}

/*
 * Copyright (C) 2003-2009 The Music Player Daemon Project
 * http://www.musicpd.org
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
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "options.h"
#include "mpc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ERROR_UNKNOWN_OPTION    0x01
#define ERROR_BAD_ARGUMENT      0x02
#define ERROR_GOT_ARGUMENT      0x03
#define ERROR_MISSING_ARGUMENT  0x04

typedef struct {
	int shortopt;
	const char *longopt;
	const char *argument;
	const char *descrition;
} arg_opt_t;


options_t options = {
	.verbosity = V_DEFAULT,
	.password = NULL,
	.port_str = NULL,
};

static const arg_opt_t option_table[] = {
		{ 'v', "verbose", NULL, "Verbose output" },
		{ 'q', "quiet", NULL, "Don't print status" },
		{ 'q', "no-status", NULL, "Don't print status" },
		{ 'h', "host", "PORT", "Connect to server on host [" DEFAULT_HOST "]" },
		{ 'P', "password", "PASSWORD", "Connect to server with password" },
		{ 'p', "port", "HOST", "Connect to server on port [" DEFAULT_PORT "]" },
		{ 'f', "format", "FORMAT", "Output status with format [" DEFAULT_FORMAT "]" }
};

static const unsigned option_table_size = sizeof(option_table) / sizeof(option_table[0]);

static void
option_error(int error, const char *option, const char *arg)
{
	switch (error) {
		case ERROR_UNKNOWN_OPTION:
			fprintf(stderr, PACKAGE ": invalid option %s\n", option);
			break;
		case ERROR_BAD_ARGUMENT:
			fprintf(stderr, PACKAGE ": bad argument: %s\n", option);
			break;
		case ERROR_GOT_ARGUMENT:
			fprintf(stderr, PACKAGE ": invalid option %s=%s\n", option, arg);
			break;
		case ERROR_MISSING_ARGUMENT:
			fprintf(stderr, PACKAGE ": missing value for %s option\n", option);
			break;
		default:
			fprintf(stderr, PACKAGE ": internal error %d\n", error);
			break;
	}
	exit(EXIT_FAILURE);
}

static const arg_opt_t *
lookup_long_option(const char *l, size_t len)
{
	unsigned i;

	for (i = 0; i < option_table_size; ++i) {
		if (strncmp(l, option_table[i].longopt, len) == 0)
			return &option_table[i];
	}

	return NULL;
}

static const arg_opt_t *
lookup_short_option(int s)
{
	unsigned i;

	for (i = 0; i < option_table_size; ++i) {
		if (s == option_table[i].shortopt)
			return &option_table[i];
	}

	return NULL;
}

static void
handle_option(int c, const char *arg)
{
	switch (c) {
		case 'v':
			options.verbosity = 2;
			break;
		case 'q':
			options.verbosity = 0;
			break;
		case 'h':
			options.host = arg;
			break;
		case 'P':
			options.password = arg;
			break;
		case 'p':
			options.port_str = arg;
			break;
		case 'f':
			options.format = arg;
			break;
		default: // Should never be reached, due to lookup_*_option functions
			fprintf(stderr, "Unknown option %c = %s\n", c, arg);
			exit(EXIT_FAILURE);
			break;
	}
}

void
parse_options(int * argc_p, char ** argv)
{
	int i, optind;
	const arg_opt_t *opt = NULL;
	char * tmp;

	for (i = 1; i < *argc_p; i++) {
		const char *arg = argv[i];
		size_t len = strlen(arg);

		if (len >= 2 && arg[0] == '-') {
			if (arg[1] == '-') {
				/* arg is a long option */
				char *value;
				size_t name_len = len - 2;

				/* make sure we got an argument for the previous option */
				if( opt && opt->argument)
					option_error(ERROR_MISSING_ARGUMENT, opt->longopt, opt->argument);

				/* retrieve a option argument */
				if ((value=index(arg+2, '='))) {
					name_len = value - arg - 2;
					value++;
				}

				/* check if the option exists */
				if ((opt=lookup_long_option(arg+2, name_len)) == NULL) {
					option_error(ERROR_UNKNOWN_OPTION, arg, NULL);
				}

				/* abort if we got an argument to the option and don't want one */
				if( value && opt->argument == NULL)
					option_error(ERROR_GOT_ARGUMENT, arg, value);

				/* execute option callback */
				if (value || opt->argument == NULL) {
					handle_option(opt->shortopt, value);
					opt = NULL;
				}
			} else {
				/* arg is a short option (or several) */
				size_t j;

				for (j=1; j<len; j++) {
					/* make sure we got an argument for the previous option */
					if (opt && opt->argument)
						option_error(ERROR_MISSING_ARGUMENT,
								opt->longopt, opt->argument);

					/* check if the option exists */
					if ((opt = lookup_short_option(arg[j])) == NULL)
						option_error(ERROR_UNKNOWN_OPTION, arg, NULL);

					/* if no option argument is needed execute callback */
					if (opt->argument == NULL) {
						handle_option(opt->shortopt, NULL);
						opt = NULL;
					}
				}
			}
		} else {
			/* No '-'; arg is an option argument or command. */
			if (opt && opt->argument) {
				handle_option(opt->shortopt, arg);
				opt = NULL;
			} else {
				break;
			}
		}
	}

	if (opt && opt->argument)
		option_error(ERROR_MISSING_ARGUMENT, opt->longopt, opt->argument);

	/* Parse the password from the host */
	if ((tmp = index(options.host, '@'))) {
		options.password = tmp + 1;
		*tmp = '\0';
	}

	/* Convert port to an integer */
	if (options.port_str) {
		options.port = strtol(options.port_str, &tmp, 10);
		if (options.port < 0 || *tmp != '\0') {
			fprintf(stderr, "Port \"%s\" is not a positive integer\n", options.port_str);
			exit(EXIT_FAILURE);
		}
	}

	/* Fix argv for command processing, which wants
	   argv[1] to be the command, and so on. */
	optind = i;
	if (optind > 1) {
		for (i = optind; i < *argc_p; i++)
			argv[i-optind+1] = argv[i];

		*argc_p -= optind - 1;
	}
}

void
options_init()
{
	char *tmp;

	options.format = DEFAULT_FORMAT;
	options.port = atoi(DEFAULT_PORT);

	if ((tmp = getenv("MPD_HOST")))
		options.host = tmp;
	else
		options.host = DEFAULT_HOST;

	if ((tmp = getenv("MPD_PORT"))) {
		options.port_str = tmp;
	}

}

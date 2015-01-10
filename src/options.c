/*
 * music player command (mpc)
 * Copyright (C) 2003-2015 The Music Player Daemon Project
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
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_LONGOPT_LENGTH 32

#define ERROR_UNKNOWN_OPTION    0x01
#define ERROR_BAD_ARGUMENT      0x02
#define ERROR_GOT_ARGUMENT      0x03
#define ERROR_MISSING_ARGUMENT  0x04

typedef struct {
	int shortopt;
	const char *longopt;
	const char *argument;
	const char *description;
} arg_opt_t;


options_t options = {
	.verbosity = V_DEFAULT,
	.password = NULL,
	.port_str = NULL,
	.format = "[%name%: &[%artist% - ]%title%]|%name%|[%artist% - ]%title%|%file%",
};

static const arg_opt_t option_table[] = {
	{ 'v', "verbose", NULL, "Give verbose output" },
	{ 'q', "quiet", NULL, "Suppress status message" },
	{ 'q', "no-status", NULL, "synonym for --quiet" },
	{ 'h', "host", "<host>", "Connect to server on <host>" },
	{ 'P', "password", "<password>", "Connect to server using password <password>" },
	{ 'p', "port", "<port>", "Connect to server port <port>" },
	{ 'f', "format", "<format>", "Print status with format <format>" },
	{ 'w', "wait", NULL, "Wait for operation to finish (e.g. database update)" },
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
	for (unsigned i = 0; i < option_table_size; ++i) {
		if (strncmp(l, option_table[i].longopt, len) == 0)
			return &option_table[i];
	}

	return NULL;
}

static const arg_opt_t *
lookup_short_option(int s)
{
	for (unsigned i = 0; i < option_table_size; ++i) {
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
		options.custom_format = true;
		break;

	case 'w':
		options.wait = true;
		break;

	default: // Should never be reached, due to lookup_*_option functions
		fprintf(stderr, "Unknown option %c = %s\n", c, arg);
		exit(EXIT_FAILURE);
		break;
	}
}

void
print_option_help(void)
{
	for (unsigned i = 0; i < option_table_size; i++) {
		printf("  -%c, ", option_table[i].shortopt);
		if (option_table[i].argument)
			printf("--%s=%-*s",
				   option_table[i].longopt,
				   20 - (int) strlen(option_table[i].longopt),
				   option_table[i].argument);
		else
			printf("--%-20s ", option_table[i].longopt);
		printf("%s\n", option_table[i].description);
	}
}

void
parse_options(int * argc_p, char ** argv)
{
	int i;
	const arg_opt_t *opt = NULL;
	char * tmp;
	int cmdind = 0;
	int optind = 0;

	for (i = 1; i < *argc_p; i++) {
		const char *arg = argv[i];
		size_t len = strlen(arg);

		if (arg[0] == '-' && (arg[1] < '0' || arg[1] > '9')) {
			if (arg[1] == '-') {
				/* arg is a long option */
				size_t name_len = len - 2;

				/* if arg is "--", there can be no more options */
				if (len == 2) {
					optind = i + 1;
					if (cmdind == 0) {
						cmdind = optind;
					}
					break;
				}

				/* make sure we got an argument for the previous option */
				if( opt && opt->argument)
					option_error(ERROR_MISSING_ARGUMENT, opt->longopt, opt->argument);

				/* retrieve a option argument */
				char *value;
				if ((value = strchr(arg + 2, '=')) != NULL) {
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
				if (len == 1)
					option_error(ERROR_UNKNOWN_OPTION, arg, NULL);

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
			/* No '-' */
			if (opt && opt->argument) {
				/* arg is an option argument */
				handle_option(opt->shortopt, arg);
				opt = NULL;
			} else {
				/* arg may the command; note it and read for more options */
				if (cmdind == 0)
					cmdind = i;
				/* otherwise, it is the first command argument and we are done */
				else {
					optind = i;
					break;
				}
			}
		}
	}
	if (optind == 0)
		optind = i;

	if (opt && opt->argument)
		option_error(ERROR_MISSING_ARGUMENT, opt->longopt, opt->argument);

	/* Parse the password from the host */
	if (options.host != NULL &&
	    (tmp = strchr(options.host, '@')) != NULL) {
		size_t password_length = tmp - options.host;
		char *password = malloc(password_length + 1);

		memcpy(password, options.host, password_length);
		password[password_length] = 0;

		options.password = password;
		options.host = tmp + 1;
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
	if (cmdind != 0)
		argv[1] = argv[cmdind];
	if (optind > 1) {
		if ( optind == cmdind || cmdind == 0 ) {
			for (i = optind + 1; i < *argc_p; i++)
				argv[i-optind+2] = argv[i];

			*argc_p -= optind - 1;
		} else {
			for (i = optind; i < *argc_p; i++)
				argv[i-optind+2] = argv[i];

			*argc_p -= optind - 2;
		}
	}
}

// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "output.h"
#include "charset.h"
#include "options.h"
#include "util.h"
#include "args.h"
#include "search.h"
#include "status.h"
#include "path.h"
#include "Compiler.h"

#include <mpd/client.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Look up an output name and return its index.
 *
 * @return the 0-based index or -1 on error
 */
gcc_pure
static int
output_name_to_index(struct mpd_connection *conn, const char *name)
{
	int result = -1;

	mpd_send_outputs(conn);

	struct mpd_output *output;
	while ((output = mpd_recv_output(conn)) != NULL) {
		bool found = strcmp(name, mpd_output_get_name(output)) == 0;
		int id = mpd_output_get_id(output);
		mpd_output_free(output);

		if (found) {
			result = id;
			break;
		}
	}

	my_finishCommand(conn);
	return result;
}

/**
 * Convert an output specification (id or name) to an index.
 *
 * @return the 0-based index or -1 on error
 */
gcc_pure
static int
output_spec_to_index(struct mpd_connection *conn, const char *spec)
{
	int result;
	if (parse_int(spec, &result)) {
		if (result <= 0)
			return -1;

		/* We decrement by 1 to make it natural to the user. */
		return result - 1;
	} else
		return output_name_to_index(conn, spec);
}

int
cmd_outputs(gcc_unused int argc, gcc_unused char **argv,
	    struct mpd_connection *conn)
{
	mpd_send_outputs(conn);

	struct mpd_output *output;
	while ((output = mpd_recv_output(conn)) != NULL) {
		/* We increment by 1 to make it natural to the user  */
		int id = mpd_output_get_id(output) + 1;
		const char *name = mpd_output_get_name(output);

		if (mpd_output_get_enabled(output)) {
			printf("Output %i (%s) is enabled\n", id, name);
		} else {
			printf("Output %i (%s) is disabled\n", id, name);
		}

		for (const struct mpd_pair *i = mpd_output_first_attribute(output);
		     i != NULL; i = mpd_output_next_attribute(output))
			printf("\t%s=\"%s\"\n", i->name, i->value);

		mpd_output_free(output);
	}

	my_finishCommand(conn);
	return 0;
}

static unsigned
match_outputs(struct mpd_connection *conn,
	      char **names, char **names_end, unsigned **ids_end)
{
	unsigned max = 0, *id = *ids_end;

	mpd_send_outputs(conn);

	struct mpd_output *output;
	while ((output = mpd_recv_output(conn)) != NULL) {
		const char *name = mpd_output_get_name(output);
		max = mpd_output_get_id(output);

		for (char **n = names; n != names_end; ++n) {
			if (!strcmp(*n, name)) {
				*id = max;
				++id;
				*n = *names;
				++names;
				break;
			}
		}

		mpd_output_free(output);
	}

	my_finishCommand(conn);

	for (char **n = names; n != names_end; ++n) {
		fprintf(stderr, "%s: no such output\n", *n);
	}

	*ids_end = id;
	return max;
}

static int
enable_disable(int argc, char **argv, struct mpd_connection *conn,
	       bool (*matched)(struct mpd_connection *conn, unsigned id),
	       bool (*not_matched)(struct mpd_connection *conn, unsigned id))
{
	char **names = argv, **names_end = argv;

	bool only = false;
	if (not_matched != NULL && !strcmp(argv[0], "only")) {
		only = true;
		++argv;
		if (!--argc) {
			DIE("No outputs specified.");
		}
	}

	unsigned *ids = malloc(argc * sizeof *ids);
	unsigned *ids_end = ids;

	for (int i = argc; i; --i, ++argv) {
		int arg;
		if (!parse_int(*argv, &arg)) {
			*names_end = *argv;
			++names_end;
		} else if (arg <= 0) {
			fprintf(stderr, "%s: not a positive integer\n", *argv);
		} else {
			/* We decrement by 1 to make it natural to the user. */
			*ids_end++ = arg - 1;
		}
	}

	unsigned max;
	if (only || names != names_end) {
		max = match_outputs(conn, names, names_end, &ids_end);
	}

	if (ids == ids_end) {
		goto done;
	}

	if (!mpd_command_list_begin(conn, false)) {
		printErrorAndExit(conn);
	}

	if (only) {
		for (unsigned i = 0; i <= max; ++i) {
			bool found = false;
			for (unsigned *id = ids;
			     !found && id != ids_end;
			     ++id) {
				found = *id == i;
			}
			(found ? matched : not_matched)(conn, i);
		}
	} else {
		for (unsigned *id = ids; id != ids_end; ++id) {
			matched(conn, *id);
		}
	}

	if (!mpd_command_list_end(conn) || !mpd_response_finish(conn)) {
		printErrorAndExit(conn);
	}

	cmd_outputs(0, NULL, conn);

done:
	free(ids);
	return 0;
}

int
cmd_enable(int argc, char **argv, struct mpd_connection *conn)
{
	return enable_disable(argc, argv, conn, mpd_send_enable_output,
			      mpd_send_disable_output);
}

int
cmd_disable(gcc_unused int argc, char **argv, struct mpd_connection *conn)
{
	return enable_disable(argc, argv, conn, mpd_send_disable_output,
			      mpd_send_enable_output);
}

int
cmd_toggle_output(int argc, char **argv, struct mpd_connection *conn)
{
	return enable_disable(argc, argv, conn, mpd_send_toggle_output,
			      mpd_send_toggle_output);
}

#ifndef HAVE_STRNDUP
static char *
strndup(const char *s, size_t length)
{
	char *p = malloc(length + 1);
	if (p != NULL) {
		memcpy(p, s, length);
		p[length] = 0;
	}

	return p;
}
#endif

int
cmd_outputset(gcc_unused int argc, char **argv, struct mpd_connection *conn)
{
	const char *const output_spec = argv[0];
	const char *const attribute = argv[1];

	int output_index = output_spec_to_index(conn, output_spec);
	if (output_index < 0)
		DIE("No such output: %s\n", output_spec);

	const char *eq = strchr(attribute, '=');
	if (eq == NULL || eq == attribute)
		DIE("Invalid attribute name/value pair: %s\n", attribute);

	char *attribute_name = strndup(attribute, eq - attribute);
	const char *attribute_value = eq + 1;

	mpd_run_output_set(conn, output_index,
			   attribute_name, attribute_value);
	free(attribute_name);
	my_finishCommand(conn);
	return 0;
}

int
cmd_moveoutput(gcc_unused int argc, char **argv, struct mpd_connection *conn)
{
	const char *name = argv[0];
	int index;
	if (parse_int(name, &index)) {
		/* We decrement by 1 to make it natural to the user. */
		index--;

		// Look up the name for the index.
		if (!mpd_send_outputs(conn)) {
			printErrorAndExit(conn);
		}

		struct mpd_output *output;
		while ((output = mpd_recv_output(conn)) != NULL) {
			if (mpd_output_get_id(output) == (unsigned)index) {
				name = mpd_output_get_name(output);
				my_finishCommand(conn);
				break;
			}

			mpd_output_free(output);
		}
	}

	/* Switch to the partition in the argument (if any) */
	if (options.partition != NULL) {
		if (!mpd_run_switch_partition(conn, options.partition)) {
			printErrorAndExit(conn);
		}
	}

	if (!mpd_run_move_output(conn, name)) {
		printErrorAndExit(conn);
	}

	return 0;
}

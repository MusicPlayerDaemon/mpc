// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "binary.h"
#include "charset.h"
#include "util.h"
#include "Compiler.h"

#include <mpd/client.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

static bool
send_binary(struct mpd_connection *connection, const char *cmd,
	    const char *uri_utf8, uint_least32_t offset)
{
	char offset_buffer[32];
	snprintf(offset_buffer, sizeof(offset_buffer),
		 "%" PRIuLEAST32, offset);

	return mpd_send_command(connection, cmd, uri_utf8,
				offset_buffer, NULL);
}

static int
cmd_binary(const char *cmd, const char *uri_utf8,
	   struct mpd_connection *connection)
{
	char *buffer = NULL;
	size_t buffer_size = 0;

	uint_least32_t offset = 0;
	while (true) {
		if (!send_binary(connection, cmd, uri_utf8, offset))
			printErrorAndExit(connection);

		struct mpd_pair *pair = mpd_recv_pair_named(connection, "binary");
		if (pair == NULL) {
			my_finishCommand(connection);
			fprintf(stderr, "No data\n");
			return 1;
		}

		char *endptr;
		uint_least32_t size = strtoul(pair->value, &endptr, 10);
		mpd_return_pair(connection, pair);
		if (endptr == pair->value || *endptr != 0) {
			my_finishCommand(connection);
			fprintf(stderr, "No data\n");
			return 1;
		}

		if (size == 0)
			/* end of file */
			break;

		if (size > buffer_size) {
			if (size > 1024U * 1024U) {
				fprintf(stderr, "Too large\n");
				return 1;
			}

			free(buffer);
			buffer = malloc(size);
			buffer_size = size;
		}

		if (!mpd_recv_binary(connection, buffer, size))
			printErrorAndExit(connection);

		my_finishCommand(connection);

		size_t nbytes = fwrite(buffer, 1, size, stdout);
		if (nbytes != size) {
			fprintf(stderr, "Write error\n");
			return 1;
		}

		offset += size;
	}
	return 0;
}

int
cmd_albumart(gcc_unused int argc, char **argv,
	     struct mpd_connection *connection)
{
	return cmd_binary("albumart", charset_to_utf8(argv[0]), connection);
}

int
cmd_readpicture(gcc_unused int argc, char **argv,
		struct mpd_connection *connection)
{
	return cmd_binary("readpicture", charset_to_utf8(argv[0]), connection);
}

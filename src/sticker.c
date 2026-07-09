// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "sticker.h"
#include "util.h"

#include <mpd/client.h>

#include <string.h>
#include <stdio.h>

static void
recv_print_stickers(struct mpd_connection *connection)
{
	struct mpd_pair *pair;

	while ((pair = mpd_recv_sticker(connection)) != NULL) {
		printf("%s=%s\n", pair->name, pair->value);
		mpd_return_sticker(connection, pair);
	}
}

/*
static void
recv_print_stickers2(struct mpd_connection *connection)
{
	struct mpd_pair *pair;
	bool first = true;

	while ((pair = mpd_recv_pair(connection)) != NULL) {
		if (strcmp(pair->name, "file") == 0) {
			if (first)
				first = false;
			else
				putchar('\n');
			printf("%s:", pair->value);
		} else if (!first && strcmp(pair->name, "sticker") == 0)
			printf(" %s", pair->value);

		mpd_return_pair(connection, pair);
	}

	if (!first)
		putchar('\n');
}
*/

static void
recv_print_stickers3(struct mpd_connection *connection) {

	struct mpd_pair *pair;
	while ((pair = mpd_recv_pair(connection)) != NULL) {

		if (strcmp(pair->name, "sticker")) {
			printf("%s: ", pair->value);
		}
		else {
			printf("%s\n", pair->value);
		}
		mpd_return_pair(connection, pair);
	}
}

int
cmd_sticker(int argc, char **argv, struct mpd_connection *conn)
{
	const char* uri = argv[0];
	const char* command = argv[1];
	//printf("argc = %d, command = %s, uri = %s\n", argc, command, uri);

	// "song" entity
	if (!strcmp(command, "set")) {
		
		if (argc < 4) {
			fputs("syntax: sticker <uri> set <name> <value>\n", stderr);
			return 0;
		}
		mpd_send_sticker_set(conn, "song", uri, argv[2], argv[3]);
	}
	else if (!strcmp(command, "get")) {

		if (argc < 3) {
			fputs("syntax: sticker <uri> get <name>\n", stderr);
			return 0;
		}
		mpd_send_sticker_get(conn, "song", uri, argv[2]);
		recv_print_stickers(conn);
	}
	else if (!strcmp(command, "find")) {

		if (argc < 3) {
			fputs("syntax: sticker <uri> find <name>\n", stderr);
			return 0;
		}
		mpd_send_sticker_find(conn, "song", uri, argv[2]);
		recv_print_stickers3(conn);
	}
	else if (!strcmp(command, "delete")) {

		if (argc < 3) {
			fputs("syntax: sticker <uri> delete <name>\n", stderr);
			return 0;
		}
		mpd_send_sticker_delete(conn, "song", uri, argv[2]);
	}
	else if (!strcmp(command, "list")) {

		if (argc < 2) {
			fputs("syntax: sticker <uri> list\n", stderr);
			return 0;
		}
		mpd_send_sticker_list(conn, "song", uri);
		recv_print_stickers(conn);
	}
	else if (!strcmp(command, "inc")) {

		if (argc < 4) {
			fputs("syntax: sticker <uri> inc <name> <value>\n", stderr);
			return 0;
		}
		unsigned value;
		sscanf(argv[3], "%u", &value);
		mpd_send_sticker_inc(conn, "song", uri, argv[2], value);
	}
	else if (!strcmp(command, "dec")) {

		if (argc < 4) {
			fputs("syntax: sticker <uri> dec <name> <value>\n", stderr);
			return 0;
		}
		unsigned value;
		sscanf(argv[3], "%u", &value);
		mpd_send_sticker_dec(conn, "song", uri, argv[2], value);
	}
	// "playlist" entity
	else if (!strcmp(command, "playlist-set")) {
		
		if (argc < 4) {
			fputs("syntax: sticker <uri> playlist-set <name> <value>\n", stderr);
			return 0;
		}
		mpd_send_sticker_set(conn, "playlist", uri, argv[2], argv[3]);
	}
	else if (!strcmp(command, "playlist-get")) {

		if (argc < 3) {
			fputs("syntax: sticker <uri> playlist-get <name>\n", stderr);
			return 0;
		}
		mpd_send_sticker_get(conn, "playlist", uri, argv[2]);
		recv_print_stickers(conn);
	}
	else if (!strcmp(command, "playlist-find")) {

		if (argc < 3) {
			fputs("syntax: sticker <uri> playlist-find <name>\n", stderr);
			return 0;
		}
		mpd_send_sticker_find(conn, "playlist", uri, argv[2]);
		recv_print_stickers3(conn);
	}
	else if (!strcmp(command, "playlist-delete")) {

		if (argc < 3) {
			fputs("syntax: sticker <uri> playlist-delete <name>\n", stderr);
			return 0;
		}
		mpd_send_sticker_delete(conn, "playlist", uri, argv[2]);
	}
	else if (!strcmp(command, "playlist-list")) {

		if (argc < 2) {
			fputs("syntax: sticker <uri> playlist-list\n", stderr);
			return 0;
		}
		mpd_send_sticker_list(conn, "playlist", uri);
		recv_print_stickers(conn);
	}
	else if (!strcmp(command, "playlist-inc")) {

		if (argc < 4) {
			fputs("syntax: sticker <uri> playlist-inc <name> <value>\n", stderr);
			return 0;
		}
		unsigned value;
		sscanf(argv[3], "%u", &value);
		mpd_send_sticker_inc(conn, "playlist", uri, argv[2], value);
	}
	else if (!strcmp(command, "playlist-dec")) {

		if (argc < 4) {
			fputs("syntax: sticker <uri> playlist-dec <name> <value>\n", stderr);
			return 0;
		}
		unsigned value;
		sscanf(argv[3], "%u", &value);
		mpd_send_sticker_dec(conn, "playlist", uri, argv[2], value);
	}
	// MPD "tag" entity
	else if (!strcmp(command, "tag-set")) {
		
		if (argc < 5) {
			fputs("syntax: sticker <uri> tag-set <tag> <name> <value>\n", stderr);
			return 0;
		}
		mpd_send_sticker_set(conn, argv[2], uri, argv[3], argv[4]);
	}
	else if (!strcmp(command, "tag-get")) {

		if (argc < 4) {
			fputs("syntax: sticker <uri> tag-get <tag> <name>\n", stderr);
			return 0;
		}
		mpd_send_sticker_get(conn, argv[2], uri, argv[3]);
		recv_print_stickers(conn);
	}
	else if (!strcmp(command, "tag-find")) {

		if (argc < 4) {
			fputs("syntax: sticker <uri> tag-find <tag> <name>\n", stderr);
			return 0;
		}
		mpd_send_sticker_find(conn, argv[2], uri, argv[3]);
		recv_print_stickers3(conn);
	}
	else if (!strcmp(command, "tag-delete")) {

		if (argc < 4) {
			fputs("syntax: sticker <uri> tag-delete <tag> <name>\n", stderr);
			return 0;
		}
		mpd_send_sticker_delete(conn, argv[2], uri, argv[3]);
	}
	else if (!strcmp(command, "tag-list")) {

		if (argc < 3) {
			fputs("syntax: sticker <uri> tag-list <tag>\n", stderr);
			return 0;
		}
		mpd_send_sticker_list(conn, argv[2], uri);
		recv_print_stickers(conn);
	}
	else if (!strcmp(command, "tag-inc")) {

		if (argc < 5) {
			fputs("syntax: sticker <uri> tag-inc <tag> <name> <value>\n", stderr);
			return 0;
		}
		unsigned value;
		sscanf(argv[4], "%u", &value);
		mpd_send_sticker_inc(conn, argv[2], uri, argv[3], value);
	}
	else if (!strcmp(command, "tag-dec")) {

		if (argc < 5) {
			fputs("syntax: sticker <uri> tag-dec <tag> <name> <value>\n", stderr);
			return 0;
		}
		unsigned value;
		sscanf(argv[4], "%u", &value);
		mpd_send_sticker_dec(conn, argv[2], uri, argv[3], value);
	}
	else {
		fprintf(stderr, "error: unknown command '%s'.\n", command);
		return 0;
	}

	my_finishCommand(conn);
	return 0;
}

int
cmd_stickernames(int argc, char **argv, struct mpd_connection *conn)
{
	(void)argc; // silence warning about unused argument
	(void)argv; // silence warning about unused argument
	struct mpd_pair *pair;

	mpd_send_stickernames(conn);

	while ((pair = mpd_recv_pair(conn)) != NULL) {

		if (!strcmp(pair->name, "name")) {
			printf("%s\n", pair->value);
		}
		mpd_return_pair(conn, pair);
	}

	my_finishCommand(conn);
	return 0;
}

int
cmd_stickertypes(int argc, char **argv, struct mpd_connection *conn)
{
	(void)argc; // silence warning about unused argument
	(void)argv; // silence warning about unused argument
	struct mpd_pair *pair;

	mpd_send_stickertypes(conn);

	while ((pair = mpd_recv_pair(conn)) != NULL) {

		if (!strcmp(pair->name, "stickertype")) {
			printf("%s\n", pair->value);
		}
		mpd_return_pair(conn, pair);
	}

	my_finishCommand(conn);
	return 0;
}

int
cmd_stickernamestypes(int argc, char **argv, struct mpd_connection *conn)
{
	(void)argc; // silence warning about unused argument
	const char* type = argv[0];
	struct mpd_pair *pair;

	mpd_send_stickernamestypes(conn, type);

	while ((pair = mpd_recv_pair(conn)) != NULL) {

		if (!strcmp(pair->name, "name")) {
			printf("%s\n", pair->value);
		}
		mpd_return_pair(conn, pair);
	}

	my_finishCommand(conn);
	return 0;
}

int
cmd_searchsticker(int argc, char **argv, struct mpd_connection *conn)
{
	(void)argc; // silence warning about unused argument
	const char* type = argv[0];
	const char* base_uri = argv[1];
	const char* name = argv[2];
	const char* oper = argv[3];
	const char* value = argv[4];

	enum mpd_sticker_operator sticker_op;

	// reverse get_sticker_oper_str() !
	if (!strcmp(oper, "="))
		sticker_op = MPD_STICKER_OP_EQ;
	else if (!strcmp(oper, ">"))
		sticker_op = MPD_STICKER_OP_GT;
	else if (!strcmp(oper, "<"))
		sticker_op = MPD_STICKER_OP_LT;
	else if (!strcmp(oper, "eq"))
		sticker_op = MPD_STICKER_OP_EQ_INT;
	else if (!strcmp(oper, "gt"))
		sticker_op = MPD_STICKER_OP_GT_INT;
	else if (!strcmp(oper, "lt"))
		sticker_op = MPD_STICKER_OP_LT_INT;
	else if (!strcmp(oper, "contains"))
		sticker_op = MPD_STICKER_OP_CONTAINS;
	else if (!strcmp(oper, "starts_with"))
		sticker_op = MPD_STICKER_OP_STARTS_WITH;
	else {
		fprintf(stderr, "error: unknown operator %s.\n", oper);
		return 0;
	}

	mpd_sticker_search_begin(conn, type, base_uri, name);

	mpd_sticker_search_add_value_constraint(conn, sticker_op, value);

	if (!mpd_sticker_search_commit(conn))
		printErrorAndExit(conn);

	recv_print_stickers3(conn);
	my_finishCommand(conn);
	return 0;
}

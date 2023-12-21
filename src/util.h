// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef MPC_UTIL_H
#define MPC_UTIL_H

#include <mpd/client.h>

struct mpd_connection;
struct mpd_song;

#define DIE(...) do { fprintf(stderr, __VA_ARGS__); return -1; } while(0)

#define SIMPLE_CMD(funcname, libmpdclient_funcname, ret) \
int funcname(gcc_unused int argc, gcc_unused char **argv, \
	     struct mpd_connection *conn) { \
	if (!libmpdclient_funcname(conn)) \
		printErrorAndExit(conn); \
	return ret; \
}

#define SIMPLE_ONEARG_CMD(funcname, libmpdclient_funcname, ret) \
int funcname (gcc_unused int argc, char **argv, struct mpd_connection *conn) { \
	if (!libmpdclient_funcname(conn, charset_to_utf8(argv[0]))) \
		printErrorAndExit(conn); \
	return ret; \
}

void
printErrorAndExit(struct mpd_connection *conn);

/**
 * Call mpd_response_finish(), and if that fails, call
 * printErrorAndExit().
 */
void
my_finishCommand(struct mpd_connection *conn);

struct mpd_status *
getStatus(struct mpd_connection *conn);

void
pretty_print_song(const struct mpd_song *song);

/**
 * @param pretty pretty-print songs (with the song format) or print
 * just the URI?
 */
void
print_entity_list(struct mpd_connection *c, enum mpd_entity_type filter_type,
		  bool pretty);

void
print_filenames(struct mpd_connection *conn);

#endif /* MPC_UTIL_H */	

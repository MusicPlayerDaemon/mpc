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

#include "libmpdclient.h"
#include "list.h"
#include "charset.h"
#include "password.h"
#include "util.h"
#include "status.h"
#include "command.h"
#include "mpc.h"
#include "options.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>

static struct command {
	const char *command;
	const int min, max;   /* min/max arguments allowed, -1 = unlimited */
	int pipe;             /**
	                       * 1: implicit pipe read, `-' optional as argv[2]
	                       * 2: explicit pipe read, `-' needed as argv[2]
	                       *
	                       * multipled by -1 if used, so that it can signal
	                       * a free() before the program exits
	                       */
	cmdhandler handler;
	const char *usage;

	/** NULL means they won't be shown in help */
	const char *help;
} mpc_table [] = {
	/* command,     min, max, pipe, handler,         usage, help */
	{"add",         0,   -1,  1,    cmd_add,         "<file>", "Add a song to the current playlist"},
	{"crop",        0,   0,   0,    cmd_crop,        "", "Remove all but the currently playing song"},
	{"del",         0,   -1,  1,    cmd_del,         "<position>", "Remove a song from the current playlist"},
	{"play",        0,   -1,  2,    cmd_play,        "[<position>]", "Start playing at <position> (default: 1)"},
	{"next",        0,   0,   0,    cmd_next,        "", "Play the next song in the current playlist"},
	{"prev",        0,   0,   0,    cmd_prev,        "", "Play the previous song in the current playlist"},
	{"pause",       0,   0,   0,    cmd_pause,       "", "Pauses the currently playing song"},
	{"toggle",      0,   0,   0,    cmd_toggle,      "", "Toggles Play/Pause, plays if stopped"},
	{"stop",        0,   0,   0,    cmd_stop,        "", "Stop the currently playing playlists"},
	{"seek",        1,   1,   0,    cmd_seek,        "[+-][HH:MM:SS]|<0-100>%", "Seeks to the specified position"},
	{"clear",       0,   0,   0,    cmd_clear,       "", "Clear the current playlist"},
	{"outputs",     0,   0,   0,    cmd_outputs,     "", "Show the current outputs"},
	{"enable",      1,   1,   0,    cmd_enable,      "<output #>", "Enable a output"},
	{"disable",     1,   1,   0,    cmd_disable,     "<output #>", "Disable a output"},
	{"shuffle",     0,   0,   0,    cmd_shuffle,     "", "Shuffle the current playlist"},
	{"move",        2,   2,   0,    cmd_move,        "<from> <to>", "Move song in playlist"},
	{"playlist",    0,   0,   0,    cmd_playlist,    "", "Print the current playlist"},
	{"listall",     0,   -1,  2,    cmd_listall,     "[<file>]", "List all songs in the music dir"},
	{"ls",          0,   -1,  2,    cmd_ls,          "[<directory>]", "List the contents of <directory>"},
	{"lsplaylists", 0,   -1,  2,    cmd_lsplaylists, "", "Lists currently available playlists"},
	{"load",        0,   -1,  1,    cmd_load,        "<file>", "Load <file> as a playlist"},
	{"save",        1,   1,   0,    cmd_save,        "<file>", "Saves a playlist as <file>"},
	{"rm",          1,   1,   0,    cmd_rm,          "<file>", "Removes a playlist"},
	{"volume",      0,   1,   0,    cmd_volume,      "[+-]<num>", "Sets volume to <num> or adjusts by [+-]<num>"},
	{"repeat",      0,   1,   0,    cmd_repeat,      "<on|off>", "Toggle repeat mode, or specify state"},
	{"random",      0,   1,   0,    cmd_random,      "<on|off>", "Toggle random mode, or specify state"},
	{"search",      2,   -1,  0,    cmd_search,      "<type> <query>", "Search for a song"},
	{"find",        2,   -1,  0,    cmd_find,        "<type> <query>", "Find a song (exact match)"},
	{"list",        1,   -1,  0,    cmd_list,        "<type> [<type> <query>]", "Show all tags of <type>"},
	{"crossfade",   0,   1,   0,    cmd_crossfade,   "[<seconds>]", "Set and display crossfade settings"},
	{"update",      0,   -1,  2,    cmd_update,      "[<path>]", "Scans music directory for updates"},
	{"stats",       0,   -1,  0,    cmd_stats,       "", "Displays statistics about MPD"},
	{"version",     0,   0,   0,    cmd_version,     "", "Reports version of MPD"},
	/* loadtab, lstab, and tab used for completion-scripting only */
	{"loadtab",     0,   1,   0,    cmd_loadtab,     "<directory>", NULL},
	{"lstab",       0,   1,   0,    cmd_lstab,       "<directory>", NULL},
	{"tab",         0,   1,   0,    cmd_tab,         "<path>", NULL},
	/* status was added for pedantic reasons */
	{"status",      0,   -1,  0,    cmd_status,      "", NULL},
	/* don't remove this, when mpc_table[i].command is NULL it will terminate the loop */
	{ .command = NULL }
};

static int print_help(char * progname, char * command)
{
	int i, max = 0;
	int ret = EXIT_FAILURE;
	FILE *outfp = stderr;

	if (command) {
		if (!strcmp(command,"help")) {
			outfp = stdout;
			ret = EXIT_SUCCESS;
		} else
			fprintf(outfp,"unknown command \"%s\"\n",command);
	}
	fprintf(outfp,"Usage: %s <command> [command args]...\n"
		"mpc version: "VERSION"\n",progname);

	for (i=0; mpc_table[i].command; ++i) {
		if (mpc_table[i].help) {
			int tmp = strlen(mpc_table[i].command) +
					strlen(mpc_table[i].usage);
			max = (tmp > max) ? tmp : max;
		}
	}

	fprintf(outfp,	"%s %*s  Displays status\n",progname,max," ");

	for (i=0; mpc_table[i].command; ++i) {
		int spaces;

		if (!mpc_table[i].help)
			continue ;
		spaces = max-(strlen(mpc_table[i].command)+strlen(mpc_table[i].usage));
		spaces += !spaces ? 0 : 1;

		fprintf(outfp,"%s %s %s%*s%s\n",progname,
			mpc_table[i].command,mpc_table[i].usage,
			spaces," ",mpc_table[i].help);

	}
	fprintf(outfp,"For more information about these and other "
			"options look at man 1 mpc\n");
	return ret;
}

static mpd_Connection * setup_connection (void)
{
	const char * host = DEFAULT_HOST;
	const char * port = DEFAULT_PORT;
	int iport;
	char * test;
	int port_env = 0;
	int host_env = 0;
	int password_len= 0;
	int parsed_len = 0;
	mpd_Connection * conn;

	if((test = getenv("MPD_HOST"))) {
		host =test;
		host_env = 1;
	}

	if((test = getenv("MPD_PORT"))) {
		port = test;
		port_env = 1;
	}

	iport = strtol(port,&test,10);

	if(iport<0 || *test!='\0') {
		fprintf(stderr,"MPD_PORT \"%s\" is not a positive integer\n",
				port);
		exit(EXIT_FAILURE);
	}

	parse_password(host, &password_len, &parsed_len);

	conn = mpd_newConnection(host+parsed_len,iport,10);

	if(conn->error && (!port_env || !host_env))
		fprintf(stderr,"MPD_HOST and/or MPD_PORT environment variables"
			" are not set\n");

	printErrorAndExit(conn);

	if(password_len)
		send_password (host, password_len, conn);

	return conn;
}

static struct command *
find_command(const char *name)
{
	for (unsigned i = 0; mpc_table[i].command != NULL; ++i)
		if (strcmp(name, mpc_table[i].command) == 0)
			return &mpc_table[i];

	return NULL;
}

/* check arguments to see if they are valid */
static char **
check_args(struct command *command, int * argc, char ** argv)
{
	char ** array;
	int i;

	if ((command->pipe == 1 &&
		(2==*argc || (3==*argc && 0==strcmp(argv[2],STDIN_SYMBOL) )))
	    || (command->pipe == 2 && (3 == *argc &&
				       0 == strcmp(argv[2],STDIN_SYMBOL)))){
		*argc = stdinToArgArray(&array);
		command->pipe *= -1;
	} else {
		*argc -= 2;
		array = malloc( (*argc * (sizeof(char *))));
		for(i=0;i<*argc;++i) {
			array[i]=argv[i+2];
		}
	}
	if ((-1 != command->min && *argc < command->min) ||
	    (-1 != command->max && *argc > command->max)) {
		fprintf(stderr,"usage: %s %s %s\n", argv[0], command->command,
			command->usage);
			exit (EXIT_FAILURE);
	}
	return array;
}

static int
run(const struct command *command, int argc, char **array)
{
	int ret;
	mpd_Connection *conn;

	conn = setup_connection();

	ret = command->handler(argc, array, conn);
	if (ret != 0) {
		struct mpc_option* nostatus = get_option("no-status");
		if (!nostatus->set)
			print_status(conn);
	}

	mpd_closeConnection(conn);
	return (ret >= 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char ** argv)
{
	int ret;
	const char *command_name;
	struct command *command;

	if(parse_options(&argc, argv) < 0)
		return print_help(argv[0],NULL);

	/* parse command and arguments */

	if (argc >= 2)
		command_name = argv[1];
	else {
		command_name = "status";

		/* this is a hack, so check_args() won't complain; the
		   arguments won't we used anyway, so this is quite
		   safe */
		argc = 2;
	}

	command = find_command(command_name);
	if (command == NULL)
		return print_help(argv[0], argv[1]);

	argv = check_args(command, &argc, argv);

	/* initialization */

	charset_init(command->pipe >= 0 || isatty(STDIN_FILENO),
		     isatty(STDOUT_FILENO));

	/* run */

	ret = run(command, argc, argv);

	/* cleanup */

	charset_deinit();

	if (command->pipe < 0)
		free_pipe_array(argc, argv);
	free(argv);

	return ret;
}

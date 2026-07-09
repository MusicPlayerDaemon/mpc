// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "list.h"
#include "binary.h"
#include "charset.h"
#include "password.h"
#include "util.h"
#include "args.h"
#include "status.h"
#include "command.h"
#include "queue.h"
#include "output.h"
#include "sticker.h"
#include "tab.h"
#include "idle.h"
#include "message.h"
#include "mount.h"
#include "neighbors.h"
#include "search.h"
#include "mpc.h"
#include "options.h"

#include <mpd/client.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __unix__
#include <sys/param.h>
#include <unistd.h>
#endif

static const struct command {
	const char *command;
	const int min, max;   /* min/max arguments allowed, -1 = unlimited */
	int pipe;             /**
	                       * 1: implicit pipe read, `-' optional as argv[2]
	                       * 2: explicit pipe read, `-' needed as argv[2]
						   * 3: implicit pipe read, `-' optional as argv[3]
	                       */
	cmdhandler handler;
	const char *usage;

	/** NULL means they won't be shown in help */
	const char *help;
} mpc_table [] = {
	/* command,          min, max, pipe, handler,         usage, help */
	{"add",              0, -1, 1, cmd_add,              "<uri>", "Add a song to the queue"},
	{"addplaylist",      2, -1, 3, cmd_addplaylist,      "<file> <uri> ...", "Add a song to the playlist"},
	{"albumart",         1,  1, 0, cmd_albumart,         "<uri>", "Download album art for the given song and write to stdout." },
	{"cdprev",           0,  0, 0, cmd_cdprev,           "", "Compact disk player-like previous command"},
	{"channels",         0,  0, 0, cmd_channels,         "", "List the channels that other clients have subscribed to." },
	{"clear",            0,  0, 0, cmd_clear,            "", "Clear the queue"},
	{"clearerror",       0,  0, 0, cmd_clearerror,       "", "Clear the current error"},
	{"clearplaylist",    1,  1, 0, cmd_clearplaylist,    "<file>", "Clear the playlist"},
	{"consume",          0,  1, 0, cmd_consume,          "<on|once|off>", "Toggle consume mode, or specify state"},
	{"crop",             0,  0, 0, cmd_crop,             "", "Remove all but the currently playing song"},
	{"crossfade",        0,  1, 0, cmd_crossfade,        "[<seconds>]", "Set and display crossfade settings"},
	{"current",          0,  0, 0, cmd_current,          "", "Show the currently playing song"},
	{"del",              0, -1, 1, cmd_del,              "<position>", "Remove a song from the queue"},
	{"delpart",          1, -1, 0, cmd_partitiondelete,  "<name> ...", "Delete partition(s)"},
	{"delplaylist",      1, -1, 3, cmd_delplaylist,      "<file> <position> ...", "Remove a song from the playlist"},
	{"disable",          1, -1, 0, cmd_disable,          "[only] <output # or name> [...]", "Disable output(s)"},
	{"enable",           1, -1, 0, cmd_enable,           "[only] <output # or name> [...]", "Enable output(s)"},
	{"find",             1, -1, 0, cmd_find,             "<type> <query>", "Find a song (exact match)"},
	{"findadd",          1, -1, 0, cmd_findadd,          "<type> <query>", "Find songs and add them to the queue"},
	{"idle",             0, -1, 0, cmd_idle,             "[events]", "Idle until an event occurs" },
	{"idleloop",         0, -1, 0, cmd_idleloop,         "[events]", "Continuously idle until an event occurs" },
	{"insert",           0, -1, 1, cmd_insert,           "<uri>", "Insert a song to the queue after the current track"},
	{"list",             1, -1, 0, cmd_list,             "<type> [<type> <query>]", "Show all tags of <type>"},
	{"listall",          0, -1, 2, cmd_listall,          "[<file>]", "List all songs in the music dir"},
	{"listneighbors",    0,  2, 0, cmd_listneighbors,    "", "List neighbors." },
	{"load",             0, -1, 1, cmd_load,             "<file>", "Load <file> into the queue"},
	{"loadtab",          1,  1, 0, cmd_loadtab,          "<directory>", NULL}, /* loadtab, lstab, and tab used for completion-scripting only */
	{"ls",               0, -1, 2, cmd_ls,               "[<directory>]", "List the contents of <directory>"},
	{"lsplaylists",      0, -1, 2, cmd_lsplaylists,      "", "List currently available playlists"},
	{"lsdirs",           0, -1, 2, cmd_lsdirs,           "[<directory>]", "List subdirectories of <directory>"},
	{"lstab",            1,  1, 0, cmd_lstab,            "<directory>", NULL},
	{"makepart",         1, -1, 0, cmd_partitionmake,    "<name> ...", "Create partition(s)"},
	{"mixrampdb",        0,  1, 0, cmd_mixrampdb,        "[<dB>]", "Set and display mixrampdb settings"},
	{"mixrampdelay",     0,  1, 0, cmd_mixrampdelay,     "[<seconds>]", "Set and display mixrampdelay settings"},
	{"mount",            0,  2, 0, cmd_mount,            "[<mount-path> <storage-uri>]", "List mounts or add a new mount." },
	{"move",             2,  2, 0, cmd_move,             "<from> <to>", "Move song in queue"},
	{"moveoutput",       1,  1, 0, cmd_moveoutput,       "<output # or name>", "Move output to partition (see -a)"},
	{"mv",               2,  2, 0, cmd_move,             "<from> <to>", NULL},
	{"moveplaylist",     3,  3, 0, cmd_moveplaylist,     "<file> <from> <to>", "Move song in playlist"},
	{"next",             0,  0, 0, cmd_next,             "", "Play the next song in the queue"},
	{"outputs",          0,  0, 0, cmd_outputs,          "", "Show the current outputs"},
	{"outputset",        2,  2, 0, cmd_outputset,        "<output # or name> <name>=<value>", "Set output attributes"},
	{"partitions",       0,  0, 0, cmd_partitionlist,   "", "List partitions"},
	{"pause",            0,  0, 0, cmd_pause,            "", "Pauses the currently playing song"},
	{"pause-if-playing", 0,  0, 0, cmd_pause_if_playing, "", "Pauses the currently playing song; exits with failure if not playing"},
	{"play",             0,  1, 2, cmd_play,             "[<position>]", "Start playing at <position>"},
	{"playlist",         0,  1, 0, cmd_playlist,         "[<playlist>]", "Print <playlist>"},
	{"playlistlength",   1,  1, 0, cmd_playlistlength,   "<file>", "Show the number of songs and their total playtime (seconds) in the playlist"},
	{"prev",             0,  0, 0, cmd_prev,             "", "Play the previous song in the queue"},
	{"prio",             2, -1, 2, cmd_prio,             "<prio> <position/range> ...", "Change song priorities in the queue"},
	{"queued",	         0,  0, 0, cmd_queued,           "", "Show the next queued song"},
	{"random",           0,  1, 0, cmd_random,           "<on|off>", "Toggle random mode, or specify state"},
	{"readpicture",      1, 1, 0,  cmd_readpicture,      "<uri>", "Download a picture from the given song and write to stdout." },
	{"renplaylist",      2,  2, 0, cmd_renplaylist,      "<file> <newfile>", "Rename a playlist"},
	{"repeat",           0,  1, 0, cmd_repeat,           "<on|off>", "Toggle repeat mode, or specify state"},
	{"replaygain",       0, -1, 0, cmd_replaygain,       "[off|track|album]", "Set or display the replay gain mode" },
	{"rescan",           0, -1, 2, cmd_rescan,           "[<path>]", "Rescan music directory (including unchanged files)"},
	{"rm",               1,  1, 0, cmd_rm,               "<file>", "Remove a playlist"},
	{"save",             1,  1, 0, cmd_save,             "<file>", "Save a queue as <file>"},
	{"search",           1, -1, 0, cmd_search,           "<type> <query>", "Search for a song"},
	{"searchadd",        1, -1, 0, cmd_searchadd,        "<type> <query>", "Search songs and add them to the queue"},
	{"searchplay",       1, -1, 0, cmd_searchplay,       "<pattern>", "Find and play a song in the queue"},
	{"searchplaylist",   2,  2, 0, cmd_searchplaylist,   "<file> <expression>", "Search the playlist for songs matching the expression"},
	{"seek",             1,  1, 0, cmd_seek,             "[+-][HH:MM:SS]|<0-100>%", "Seeks to the specified position"},
	{"seekthrough",      1,  1, 0, cmd_seek_through,      "[+-][HH:MM:SS]", "Seeks by an amount of time within the song and playlist"},
	{"sendmessage",      2,  2, 0, cmd_sendmessage,      "<channel> <message>", "Send a message to the specified channel." },
	{"shuffle",          0,  0, 0, cmd_shuffle,          "", "Shuffle the queue"},
	{"single",           0,  1, 0, cmd_single,           "<on|once|off>", "Toggle single mode, or specify state"},
	{"stats",            0, -1, 0, cmd_stats,            "", "Display statistics about MPD"},
	{"status",           0, -1, 0, cmd_status,           "", NULL}, /* status was added for pedantic reasons */
	{"sticker",          2, -1, 0, cmd_sticker,          "<uri> <get[-playlist|-tag]|set[-playlist|-tag]|list[-playlist|-tag]|delete[-playlist|-tag]|find[-playlist|-tag]|inc[-playlist|-tag]|dec[-playlist|-tag]> [args..]", "Sticker management"},
	{"stickernames",     0,  0, 0, cmd_stickernames,     "", "Display the list of unique sticker names"},
	{"stickertypes",     0,  0, 0, cmd_stickertypes,     "", "Display the list of available sticker types"},
	{"stickernamestypes",1,  1, 0, cmd_stickernamestypes,"<type>", "Display the list of unique sticker names for this sticker type"},
	{"searchsticker",    5,  5, 0, cmd_searchsticker,    "<type> <uri> <name> <oper> <value>", "Search for MPD <type> entities having sticker <name> <oper> to <value> set at <uri>"},
	{"stop",             0,  0, 0, cmd_stop,             "", "Stop playback"},
	{"subscribe",        1,  1, 0, cmd_subscribe,        "<channel>", "Subscribe to the specified channel and continuously receive messages." },
	{"tab",              1,  1, 0, cmd_tab,              "<path>", NULL},
	{"tags",             0,  0, 0, cmd_tags,             "", "Display all MPD tags known by the mpc client" },
	{"toggle",           0,  0, 0, cmd_toggle,           "", "Toggles Play/Pause, plays if stopped"},
	{"toggleoutput",     1, -1, 0, cmd_toggle_output,    "<output # or name> [...]", "Toggle output(s)"},
	{"unmount",          1,  1, 0, cmd_unmount,          "<mount-path>", "Remove a mount." },
	{"update",           0, -1, 2, cmd_update,           "[<path>]", "Scan music directory for updates"},
	{"version",          0,  0, 0, cmd_version,          "", "Report version of MPD"},
	{"volume",           0,  1, 0, cmd_volume,           "[+-]<num>", "Set volume to <num> or adjusts by [+-]<num>"},
	{"waitmessage",      1,  1, 0, cmd_waitmessage,      "<channel>", "Wait for at least one message on the specified channel." },

	/* don't remove this, when mpc_table[i].command is NULL it will terminate the loop */
	{ .command = NULL }
};

static void
print_usage(FILE *outfp, const char *progname)
{
	fprintf(outfp,"Usage: %s [options] <command> [--] [<arguments>]\n"
		"mpc version: "VERSION"\n",progname);
}

static int
print_help(const char *progname, const char *command)
{
	if (command && strcmp(command, "help")) {
		fprintf(stderr,"unknown command \"%s\"\n",command);
		print_usage(stderr, progname);
		fprintf(stderr,"See man 1 mpc or use 'mpc help' for more help.\n");
		return EXIT_FAILURE;
	}

	print_usage(stdout, progname);
	printf("\n");

	printf("Options:\n");
	print_option_help();
	printf("\n");

	printf("Commands:\n");

	unsigned max = 0;
	for (unsigned i = 0; mpc_table[i].command != NULL; ++i) {
		if (mpc_table[i].help) {
			unsigned tmp = strlen(mpc_table[i].command) +
				strlen(mpc_table[i].usage);
			max = (tmp > max) ? tmp : max;
		}
	}

	printf("  %s %*s  Display status\n",progname,max," ");

	for (unsigned i = 0; mpc_table[i].command != NULL; ++i) {
		if (!mpc_table[i].help)
			continue ;

		int spaces = max-(strlen(mpc_table[i].command)+strlen(mpc_table[i].usage));
		spaces += !spaces ? 0 : 1;

		printf("  %s %s %s%*s%s\n",progname,
			mpc_table[i].command,mpc_table[i].usage,
			spaces," ",mpc_table[i].help);

	}
	printf("\nSee man 1 mpc for more information about mpc commands and options\n");
	return EXIT_SUCCESS;
}

static struct mpd_connection *
setup_connection(void)
{
	struct mpd_connection *conn = mpd_connection_new(options.host, options.port, 0);
	if (conn == NULL) {
		fputs("Out of memory\n", stderr);
		exit(EXIT_FAILURE);
	}

	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
		printErrorAndExit(conn);

	if(options.password)
		send_password(options.password, conn);

	return conn;
}

static const struct command *
find_command(const char *name)
{
	unsigned n_matches = 0;
	size_t name_length = strlen(name);
	const struct command *command = NULL;

	for (unsigned i = 0; mpc_table[i].command != NULL; ++i) {
		if (memcmp(name, mpc_table[i].command, name_length) == 0) {
			command = &mpc_table[i];
			++n_matches;

			if (command->command[name_length] == 0)
				/* exact match */
				return &mpc_table[i];
		}
	}

	return n_matches == 1
		? command
		: /* ambiguous or nonexistent */ NULL;
}

/**
 * Set to true when "argv" contains an allocated list which must be
 * freed before exiting.
 */
static bool pipe_array_used = false;

/* check arguments to see if they are valid */
static char **
check_args(const struct command *command, int * argc, char ** argv)
{
	char ** array;

	if ((command->pipe == 1 &&
		(2==*argc || (3==*argc && 0==strcmp(argv[2],STDIN_SYMBOL) )))
	    || (command->pipe == 2 && (3 == *argc &&
				       0 == strcmp(argv[2],STDIN_SYMBOL)))){
		*argc = stdinToArgArray(&array);
		pipe_array_used = true;

	} else if (command->pipe == 3 && ((*argc == 3) || ((*argc == 4) && !strcmp(argv[3], STDIN_SYMBOL)))) {
		*argc = stdinAndPreambleToArgArray(&array, argv[2]);
		pipe_array_used = true;

	} else {
		*argc -= 2;
		array = malloc( (*argc * (sizeof(char *))));
		for(int i = 0; i < *argc; ++i) {
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
	struct mpd_connection *conn = setup_connection();

	if (mpd_connection_cmp_server_version(conn, 0, 21, 0) < 0)
		fprintf(stderr, "warning: MPD 0.21 required\n");

	/* If a partition was specified, switch to it, unless we're moving
	   an output.  Not all outputs are visible in a partition, and
	   moveoutput needs to look up the one to move, so it has to start
	   in the default partition. */
	if (options.partition != NULL && command->handler != cmd_moveoutput) {
		if (!mpd_run_switch_partition(conn, options.partition)) {
			printErrorAndExit(conn);
		}
	}

	int ret = command->handler(argc, array, conn);
	if (ret > 0 && options.verbosity > V_QUIET) {
		print_status(conn);
	}

	mpd_connection_free(conn);
	return (ret >= 0) ? EXIT_SUCCESS : -ret;
}

int main(int argc, char ** argv)
{
	parse_options(&argc, argv);

	/* parse command and arguments */

	const char *command_name;
	if (argc >= 2)
		command_name = argv[1];
	else {
		command_name = "status";

		/* this is a hack, so check_args() won't complain; the
		   arguments won't we used anyway, so this is quite
		   safe */
		argc = 2;
	}

	const struct command *command = find_command(command_name);
	if (command == NULL)
		return print_help("mpc", argv[1]);

	argv = check_args(command, &argc, argv);

	/* initialization */

	charset_init(true, true);

	/* run */

	int ret = run(command, argc, argv);

	/* cleanup */

	charset_deinit();

	if (pipe_array_used)
		free_pipe_array(argc, argv);
	free(argv);

	return ret;
}

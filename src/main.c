/* mpc
 * (c) 2003-2004 by Warren Dukes (warren.dukes@gmail.com)
 *                  Daniel Brown (danb@cs.utexas.edu)
 * This project's homepage is: http://www.musicpd.org
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "libmpdclient.h"
#include "list.h"
#include "charConv.h"
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

struct _mpc_table {
	const char * command; 
	const int min, max;	/* min/max arguments allowed, -1 = unlimited */
	int pipe;		/* 
				1: implicit pipe read, `-' optional as argv[2]
				2: explicit pipe read `-' needed as argv[2]
				3: explicit pipe read `-' needed as argv[3]
				multipled by -1 if used, so that it can signal a free()
				before the program exits
				*/
	cmdhandler handler;
	const char * usage;
	char * help;		/* NULL means they won't be shown in help */
} mpc_table [] = {
	/* Command Name, Minimum Arguments, Maximium args, pipe read (see above), Argument description, command description */
	{"add", 0, -1, 1, cmd_add,"<filename>","Add a song to the current playlist" },
	{"crop", 0, 0, 0, cmd_crop,"","Remove all songs except for the currently playing song" },
	{"del", 0, -1, 1, cmd_del,"<playlist #>","Remove a song from the current playlist" },
	{"play", 0, -1, 2, cmd_play,"<number>","Start playing at <number> (default: 1)" },
	{"next", 0, 0, 0, cmd_next, "","Play the next song in the current playlist"},
	{"prev", 0, 0, 0, cmd_prev,"","Play the previous song in the current playlist"},
	{"pause", 0, 0, 0, cmd_pause, "", "Pauses the currently playing song"},
	{"toggle", 0, 0, 0, cmd_toggle, "", "Toggles Play/Pause, plays if stopped"},
	{"stop", 0, 0, 0, cmd_stop,"", "Stop the currently playing playlists"},
	{"seek", 1, 1, 0, cmd_seek,"[+-][HH:MM:SS] or <0-100>%","Seeks to the specified position"},
	{"clear", 0, 0, 0, cmd_clear,"", "Clear the current playlist"},
	/*
	{"outputs", 0, 0, 0, cmd_outputs,"", "Show the current outputs"},
	{"enable", 1, 1, 0, cmd_enable, "<output #>", "Enable a output"},
	{"disable", 1, 1, 0, cmd_disable, "<output #>", "Disable a output"},
	*/
	{"shuffle", 0, 0, 0, cmd_shuffle,"", "Shuffle the current playlist"},
	{"move", 2, 2, 0, cmd_move,"<from> <to>", "Move song in playlist"},
	{"playlist", 0, 0, 0, cmd_playlist, "", "Print the current playlist"},
	{"listall", 0, -1, 2, cmd_listall,"[<song>]","List all songs in the music dir"},
	{"ls", 0, -1, 2, cmd_ls,"[<dir>]","List the contents of <dir>"},
	{"lsplaylists", 0, -1, 2, cmd_lsplaylists,"", "Lists currently available playlists"},
	{"load", 0, -1, 1, cmd_load,"<file>","Load <file> as a playlist"},
	{"save", 1, 1, 0, cmd_save,"<file>", "Saves a playlist as <file>"},
	{"rm", 1, 1, 0, cmd_rm,"<file>","Removes a playlist"},
	{"volume", 0, 1, 0, cmd_volume,"[+-]<num>","Sets volume to <num> or adjusts by [+-]<num>"},
	{"repeat", 0, 1, 0, cmd_repeat,"<on|off>", "Toggle repeat mode, or specify state"},
	{"random", 0, 1, 0, cmd_random,"<on|off>", "Toggle random mode, or specify state"},
	{"search", 2, -1, 3, cmd_search, "<type> <queries>",	"Search for a song"},
	{"crossfade", 0, 1, 0, cmd_crossfade,"[sec]","Set and display crossfade settings"},
	{"update", 0, -1, 2, cmd_update,	"", "Scans music directory for updates"},
	{"stats", 0, -1, 0, cmd_stats, "", "Displays statistics about MPD"}, 
	{"version", 0, 0, 0, cmd_version,"", "Reports version of MPD"},
	/*  loadtab, lstab, and tab used for completion-scripting only */
	{"loadtab",0, 1, 0, cmd_loadtab,"<directory>",}, 
	{"lstab",0, 1, 0, cmd_lstab,"<directory>",}, 
	{"tab",0, 1, 0, cmd_tab,"<directory/file>",},
	/* status was added for pedantic reasons */
	{"status", 0, -1, 0, cmd_status, "", NULL }, 
	/* don't remove this, when mpc_table[i].command is NULL it will terminate the loop */
	{}
};

void print_help (int max, char * progname, char * command)
{
	int i;

	if(command && strcmp(command,"help"))
			fprintf(stderr,"unknown command \"%s\"\n",command);
	fprintf(stderr,"Usage: %s <command> [command args]...\n"
		"mpc version: "VERSION"\n",progname);
	fprintf(stderr,	"%s %*s  Displays status\n",progname,max," ");
	
	for (i=0; mpc_table[i].command; ++i) {
		int spaces;
		
		if (!mpc_table[i].help)
			continue ;
		spaces = max-(strlen(mpc_table[i].command)+strlen(mpc_table[i].usage));
		spaces += !spaces ? 0 : 1;

		fprintf(stderr,"%s %s %s%*s%s\n",progname,
			mpc_table[i].command,mpc_table[i].usage,
			spaces," ",mpc_table[i].help);
		
	}
	fprintf(stderr,"For more information about these and other options look at man 1 mpc\n");
}

mpd_Connection * setup_connection ()
{
	char * host = DEFAULT_HOST;
	char * port = DEFAULT_PORT;
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

void print_status_and_exit () 
{
	mpd_Connection * conn = setup_connection();
	print_status(conn);
	mpd_closeConnection(conn);
	fclose(stdout);
	exit(EXIT_SUCCESS);
}

/* check arguments to see if they are valid */
char ** check_args (int idx, int * argc, char ** argv)
{
	char ** array;
	int i;
	
	if ( ( mpc_table[idx].pipe==1 &&
		(2==*argc || (3==*argc && 0==strcmp(argv[2],STDIN_SYMBOL) )))
	|| (mpc_table[idx].pipe==2 && (3==*argc && 0==strcmp(argv[2],STDIN_SYMBOL)))
	|| (mpc_table[idx].pipe==3 && (4==*argc && 0==strcmp(argv[3],STDIN_SYMBOL)))
	){
		*argc = stdinToArgArray(&array);
		mpc_table[idx].pipe *= -1;
	} else {
		*argc -= 2;
		array = malloc( (*argc * (sizeof(char *))));
		for(i=0;i<*argc;++i) {
			array[i]=argv[i+2];
		}
	}
	if (	(-1!=mpc_table[idx].min && (*argc)<mpc_table[idx].min)
	||	(-1!=mpc_table[idx].max && (*argc)>mpc_table[idx].max) ) {
		fprintf(stderr,"usage: %s %s %s\n", argv[0], mpc_table[idx].command,
			mpc_table[idx].usage);
			exit (EXIT_FAILURE);
	}
	return array;
}

int main(int argc, char ** argv)
{
	int i = 0;
	int helplen = 0;
	int ret = 0;
	setLocaleCharset();

	if(parse_options(&argc, argv) < 0) {
		print_help(helplen,argv[0],NULL);
		exit(EXIT_FAILURE);
	}

	if (argc==1)
		print_status_and_exit ();

	for (i = 0; mpc_table[i].command; ++i) {
		if ( ! strcmp(argv[1], mpc_table[i].command) ) {
			char ** array = check_args (i, &argc , argv);
			mpd_Connection * conn = setup_connection();
			
			/* not a typo, assignment intended */
			if( (ret=mpc_table[i].handler(argc, array, conn)))
				print_status(conn);
			
			if (0>mpc_table[i].pipe)
				free_pipe_array(argc,array);
			
			free(array);
			mpd_closeConnection(conn);
			fclose(stdout);
			return EXIT_SUCCESS;
		} else if (mpc_table[i].help) {
			int tmp = strlen(mpc_table[i].command) + strlen(mpc_table[i].usage);
			helplen = (tmp > helplen) ? tmp : helplen;
		}
	}
	print_help(helplen,argv[0],argv[1]);
	fclose(stdout);
	return EXIT_SUCCESS;
}


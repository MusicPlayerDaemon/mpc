/* mpc
 * (c)2003-2004 by Warren Dukes (shank@mercury.chem.pitt.edu)
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <assert.h>

#define STRING_LENGTH	(2*MAXPATHLEN)

#define STDIN_SYMBOL	"-"

void printErrorAndExit(mpd_Connection * conn) {
	if(conn->error) {
		fprintf(stderr,"error: %s\n",fromUtf8(conn->errorStr));
		exit(-1);
	}
}

int stdinToArgArray(char *** array) {
	List * list = makeList(NULL);
	ListNode * node;
	char buffer[STRING_LENGTH+1];
	int size;
	int i;
	char * sp;

	while(fgets(buffer,STRING_LENGTH,stdin)) {
		if((sp = strchr(buffer,'\n'))) *sp = '\0';
		insertInListWithoutKey(list,strdup(buffer));
	}

	size = list->numberOfNodes;
	*array = malloc((sizeof(char *))*size);

	i = 0;
	node = list->firstNode;
	while(node) {
		(*array)[i++] = (char *)node->data;
		node = node->nextNode;
	}
	assert(i==size);

	freeList(list);

	return size;
}

int main(int argc, char ** argv) {
	mpd_Connection * conn;
	char * host = DEFAULT_HOST;
	char * port = DEFAULT_PORT;
	int iport;
	char * test;
	int port_env = 0;
	int host_env = 0;

	setLocaleCharset();

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
		return -1;
	}

	conn = mpd_newConnection(host,iport,10);
	if(conn->error && (!port_env || !host_env)) {
		fprintf(stderr,"MPD_HOST and/or MPD_PORT environment variables"
			" are not set\n");
	}
	printErrorAndExit(conn);

	if(argc==1) goto status;
	else if(strcmp(argv[1],"lsplaylists")==0) {
		mpd_InfoEntity * entity;
		char * ls = "";
		int i;
		int numargs;
		int offset;
		char ** args;

		if(argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0) {
			offset = 0;
			numargs = stdinToArgArray(&args);
		}
		else {
			offset = 2;
			numargs = argc;
			args = argv;
		}

		i = offset;

		if(argc>2) ls = toUtf8(args[i]);

		do {
			mpd_sendLsInfoCommand(conn,ls);
			printErrorAndExit(conn);

			while((entity = mpd_getNextInfoEntity(conn))) {
				if(entity->type==
					MPD_INFO_ENTITY_TYPE_PLAYLISTFILE) {
					mpd_PlaylistFile * pl = 
						entity->info.playlistFile;
					printf("%s\n",fromUtf8(pl->path));
				}
				mpd_freeInfoEntity(entity);
			}

			printErrorAndExit(conn);

			mpd_finishCommand(conn);
			printErrorAndExit(conn);
		} while(++i<numargs && (ls = toUtf8(args[i])));

		if(argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0) {
			for(i=0;i<numargs;i++) free(args[i]);
			free(args);
		}
	}
	else if(strcmp(argv[1],"ls")==0) {
		mpd_InfoEntity * entity;
		char * ls = "";
		int i;
		int numargs;
		int offset;
		char ** args;
		char * sp;
		char * dp;

		if(argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0) {
			offset = 0;
			numargs = stdinToArgArray(&args);
		}
		else {
			offset = 2;
			numargs = argc;
			args = argv;
		}

		i = offset;

		if(argc>2) ls = toUtf8(args[i]);

		do {
			mpd_sendListallCommand(conn,"");

			sp = ls;
			while((sp = strchr(sp,' '))) *sp = '_';

			while((entity = mpd_getNextInfoEntity(conn))) {
				if(entity->type==MPD_INFO_ENTITY_TYPE_DIRECTORY) {
					mpd_Directory * dir = entity->info.directory;
					sp = dp = strdup(dir->path);
					while((sp = strchr(sp,' '))) *sp = '_';
					if(strcmp(dp,ls)==0) {
						free(dp);
						ls = dir->path;
						break;
					}
					free(dp);
				}
				mpd_freeInfoEntity(entity);
			}

			printErrorAndExit(conn);

			mpd_finishCommand(conn);
			printErrorAndExit(conn);

			mpd_sendLsInfoCommand(conn,ls);
			printErrorAndExit(conn);

			while((entity = mpd_getNextInfoEntity(conn))) {
				if(entity->type==MPD_INFO_ENTITY_TYPE_DIRECTORY) {
					mpd_Directory * dir = entity->info.directory;
	
					printf("%s\n",fromUtf8(dir->path));
				}
				else if(entity->type==MPD_INFO_ENTITY_TYPE_SONG) {
					mpd_Song * song = entity->info.song;
	
					printf("%s\n",fromUtf8(song->file));
				}
				mpd_freeInfoEntity(entity);
			}

			printErrorAndExit(conn);

			mpd_finishCommand(conn);
			printErrorAndExit(conn);
		} while(++i<numargs && (ls = toUtf8(args[i])));

		if(argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0) {
			for(i=0;i<numargs;i++) free(args[i]);
			free(args);
		}
	}
	else if(strcmp(argv[1],"search")==0) {
		mpd_InfoEntity * entity;
		char * search;
		int table;
		int i;
		int numargs;
		int offset;
		char ** args;

		if(argc<=3) {
			fprintf(stderr,"Usage: %s search <album|artist|title|"
				"filename> <search arg1> ...\n", argv[0]);
			return -1;
		}

		if(strcmp("album",argv[2])==0) table = MPD_TABLE_ALBUM;
		else if(strcmp("artist",argv[2])==0) table = MPD_TABLE_ARTIST;
		else if(strcmp("title",argv[2])==0) table = MPD_TABLE_TITLE;
		else if(strcmp("filename",argv[2])==0) {
			table = MPD_TABLE_FILENAME;
		}
		else {
			fprintf(stderr,"%s is not one of: album, artist, title,"
				"filename\n", argv[2]);
			return -1;
		}

		if(argc==4 && strcmp(argv[3],STDIN_SYMBOL)==0) {
			offset = 0;
			numargs = stdinToArgArray(&args);
		}
		else {
			offset = 3;
			numargs = argc;
			args = argv;
		}

		i = offset;

		while(i<numargs && (search = toUtf8(args[i])))  {
			mpd_sendSearchCommand(conn,table,search);
			printErrorAndExit(conn);

			while((entity = mpd_getNextInfoEntity(conn))) {
				printErrorAndExit(conn);
				if(entity->type==MPD_INFO_ENTITY_TYPE_DIRECTORY) {
					mpd_Directory * dir = entity->info.directory;
	
					printf("%s\n",fromUtf8(dir->path));
				}
				else if(entity->type==MPD_INFO_ENTITY_TYPE_SONG) {
					mpd_Song * song = entity->info.song;
	
					printf("%s\n",fromUtf8(song->file));
				}
				mpd_freeInfoEntity(entity);
			}

			printErrorAndExit(conn);

			mpd_finishCommand(conn);
			printErrorAndExit(conn);

			i++;
		}

		if(argc==4 && strcmp(argv[3],STDIN_SYMBOL)==0) {
			for(i=0;i<numargs;i++) free(args[i]);
			free(args);
		}
	}
	else if(strcmp(argv[1],"listall")==0) {
		mpd_InfoEntity * entity;
		char * listall = "";
		int i=2;
		int numargs;
		int offset;
		char ** args;

		if(argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0) {
			offset = 0;
			numargs = stdinToArgArray(&args);
		}
		else {
			offset = 2;
			numargs = argc;
			args = argv;
		}

		i = offset;

		if(argc>2) listall = toUtf8(args[i]);

		do {
			mpd_sendListallCommand(conn,listall);
			printErrorAndExit(conn);

			while((entity = mpd_getNextInfoEntity(conn))) {
				if(entity->type==MPD_INFO_ENTITY_TYPE_SONG) {
					mpd_Song * song = entity->info.song;
	
					printf("%s\n",fromUtf8(song->file));
				}
				mpd_freeInfoEntity(entity);
			}
	
			printErrorAndExit(conn);

			mpd_finishCommand(conn);
			printErrorAndExit(conn);
		} while(++i<numargs && (listall = toUtf8(args[i])));

		if(argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0) {
			for(i=0;i<numargs;i++) free(args[i]);
			free(args);
		}
	}
	else if(strcmp(argv[1],"playlist")==0) {
		mpd_InfoEntity * entity;
		int count = 0;
		
		if(argc!=2) {
			fprintf(stderr,"usage: %s playlist\n",
					argv[0]);
			return -1;
		}

		mpd_sendPlaylistInfoCommand(conn,-1);
		printErrorAndExit(conn);

		while((entity = mpd_getNextInfoEntity(conn))) {
			if(entity->type==MPD_INFO_ENTITY_TYPE_SONG) {
				mpd_Song * song = entity->info.song;
				if(song->artist && song->title) {
					printf("#%i) %s - ",1+count,
							fromUtf8(song->artist));
					printf("%s\n",fromUtf8(song->title));
				}
				else printf("#%i) %s\n",1+count,
							fromUtf8(song->file));
				count++;
			}
			mpd_freeInfoEntity(entity);
		}

		printErrorAndExit(conn);

		mpd_finishCommand(conn);
		printErrorAndExit(conn);
	}
	else if(strcmp(argv[1],"loadtab")==0) {
		mpd_InfoEntity * entity;
		char * sp;
		mpd_PlaylistFile * pl;

		if(argc>3) {
			fprintf(stderr,"usage: %s loadtab <directory>\n",
					argv[0]);
			return -1;
		}

		mpd_sendLsInfoCommand(conn,"");
		printErrorAndExit(conn);

		while((entity = mpd_getNextInfoEntity(conn))) {
			if(entity->type==MPD_INFO_ENTITY_TYPE_PLAYLISTFILE) {
				pl = entity->info.playlistFile;
				sp = pl->path;
				while((sp = strchr(sp,' '))) *sp = '_';
				if(argc==3) {
					if(strncmp(pl->path,argv[2],
						strlen(argv[2]))==0) {
						printf("%s\n",
							fromUtf8(pl->path));
					}
				}
			}
			mpd_freeInfoEntity(entity);
		}

		printErrorAndExit(conn);

		mpd_finishCommand(conn);
		printErrorAndExit(conn);
	}
	else if(strcmp(argv[1],"lstab")==0) {
		mpd_InfoEntity * entity;
		char * sp;
		mpd_Directory * dir;

		if(argc>3) {
			fprintf(stderr,"usage: %s lstab <directory>\n",
					argv[0]);
			return -1;
		}

		mpd_sendListallCommand(conn,"");
		printErrorAndExit(conn);

		while((entity = mpd_getNextInfoEntity(conn))) {
			if(entity->type==MPD_INFO_ENTITY_TYPE_DIRECTORY) {
				dir = entity->info.directory;
				sp = dir->path;
				/* replace all ' ' with '_' */
				while((sp = strchr(sp,' '))) *sp = '_';
				if(argc==3) {
					if(strncmp(dir->path,argv[2],
						strlen(argv[2]))==0) {
						printf("%s\n",
							fromUtf8(dir->path));
					}
				}
			}
			mpd_freeInfoEntity(entity);
		}

		printErrorAndExit(conn);

		mpd_finishCommand(conn);
		printErrorAndExit(conn);
	}
	else if(strcmp(argv[1],"tab")==0) {
		mpd_InfoEntity * entity;
		char * sp;
		mpd_Song * song;

		if(argc>3) {
			fprintf(stderr,"usage: %s tab <directory/file>\n",
					argv[0]);
			return -1;
		}

		mpd_sendListallCommand(conn,"");
		printErrorAndExit(conn);

		while((entity = mpd_getNextInfoEntity(conn))) {
			if(entity->type==MPD_INFO_ENTITY_TYPE_SONG) {
				song = entity->info.song;
				sp = song->file;
				/* replace all ' ' with '_' */
				while((sp = strchr(sp,' '))) *sp = '_';
				if(argc==3) {
					if(strncmp(song->file,argv[2],
						strlen(argv[2]))==0) {
						printf("%s\n",
							fromUtf8(song->file));
					}
				}
				else printf("%s\n",fromUtf8(song->file));
			}
			mpd_freeInfoEntity(entity);
		}

		printErrorAndExit(conn);

		mpd_finishCommand(conn);
		printErrorAndExit(conn);
	}
	else if(strcmp(argv[1],"del")==0) {
		int offset;
		char ** args;
		int numargs;
		int i,j;
		char * s;
		char * t;
		char * t2;
		int range[2];
		int songsDeleted = 0;
		int plLength = 0;
		char * songsToDel;
		mpd_Status * status;
		
		if(argc==2 || (argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0)) {
			offset = 0;
			numargs = stdinToArgArray(&args);
		}
		else {
			offset = 2;
			numargs = argc;
			args = argv;
		}

		status = mpd_getStatus(conn);
		printErrorAndExit(conn);
		plLength = status->playlistLength;
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		songsToDel = malloc(plLength);
		memset(songsToDel,0,plLength);

		for(i=offset;i<numargs;i++) {
			if(args[i][0]=='#') s = &(args[i][1]);
			else s = args[i];

			range[0] = strtol(s,&t,10);
			if(s==t) {
				fprintf(stderr,"error parsing song numbers from"
					": %s\n",args[i]);
				exit(-1);
			}
			else if(*t=='-') {
				range[1] = strtol(t+1,&t2,10);
				if(t+1==t2 || *t2!='\0') {
					fprintf(stderr,"error parsing range "
						"from: %s\n",args[i]);
					exit(-1);
				}
			}
			else if(*t==')' || *t=='\0') range[1] = range[0];
			else {
				fprintf(stderr,"error parsing song numbers from"
					": %s\n",args[i]);
				exit(-1);
			}

			if(range[0]<=0 || range[1]<=0 || range[1]<range[0]) {
				fprintf(stderr,"song numbers must be positive:"
					" %i-%i\n",range[0],range[1]);
				exit(-1);
			}

			if(range[1]>plLength) {
				fprintf(stderr,"song number does not exist:"
					"%i\n",range[1]);
				exit(-1);
			}

			for(j=range[0];j<=range[1];j++) songsToDel[j-1] = 1;
		}

		mpd_sendCommandListBegin(conn);
		printErrorAndExit(conn);
		for(i=0;i<plLength;i++) {
			if(songsToDel[i]) {
				mpd_sendDeleteCommand(conn,i-songsDeleted);
				printErrorAndExit(conn);
				songsDeleted++;
			}
		}
		mpd_sendCommandListEnd(conn);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		free(songsToDel);

		if(argc==2 || (argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0)) {
			for(i=0;i<numargs;i++) free(args[i]);
			free(args);
		}
	}
	else if(strcmp(argv[1],"save")==0) {
		if(argc!=3) {
			fprintf(stderr,"usage: %s save <name>\n",
					argv[0]);
			return -1;
		}

		mpd_sendSaveCommand(conn,toUtf8(argv[2]));
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);
	}
	else if(strcmp(argv[1],"rm")==0) {
		if(argc!=3) {
			fprintf(stderr,"usage: %s rm <name>\n",
					argv[0]);
			return -1;
		}

		mpd_sendRmCommand(conn,toUtf8(argv[2]));
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);
	}
	else if(strcmp(argv[1],"load")==0) {
		int i;
		int offset;
		char ** args;
		int numargs;
		char * sp;
		char * dp;
		mpd_InfoEntity * entity;
		mpd_PlaylistFile * pl;

		if(argc==2 || (argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0)) {
			offset = 0;
			numargs = stdinToArgArray(&args);
		}
		else {
			offset = 2;
			numargs = argc;
			args = argv;
		}

		for(i=offset;i<numargs;i++) {
			sp = args[i];
			while((sp = strchr(sp,' '))) *sp = '_';
		}

		mpd_sendLsInfoCommand(conn,"");
		printErrorAndExit(conn);
		while((entity = mpd_getNextInfoEntity(conn))) {
			if(entity->type==MPD_INFO_ENTITY_TYPE_PLAYLISTFILE) {
				pl = entity->info.playlistFile;
				dp = sp = strdup(fromUtf8(pl->path));
				while((sp = strchr(sp,' '))) *sp = '_';
				for(i=offset;i<numargs;i++) {
					if(strcmp(dp,args[i])==0) {
						strcpy(args[i],
							fromUtf8(pl->path));
					}
				}
				free(dp);
				mpd_freeInfoEntity(entity);
			}
		}
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		mpd_sendCommandListBegin(conn);
		printErrorAndExit(conn);
		for(i=offset;i<numargs;i++) {
			printf("loading: %s\n",args[i]);
			mpd_sendLoadCommand(conn,toUtf8(args[i]));
			printErrorAndExit(conn);
		}
		mpd_sendCommandListEnd(conn);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		/* clean up */
		if(argc==2 || (argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0)) {
			for(i=0;i<numargs;i++) free(args[i]);
			free(args);
		}
	}
	else if(strcmp(argv[1],"add")==0) {
		List ** lists;
		mpd_InfoEntity * entity;
		ListNode * node;
		mpd_Song * song;
		char * sp;
		char * dup;
		int i;
		int offset;
		char ** args;
		int numargs;
		int * arglens;
		int len;
		int ret;

		if(argc==2 || (argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0)) {
			offset = 0;
			numargs = stdinToArgArray(&args);
		}
		else {
			offset = 2;
			numargs = argc;
			args = argv;
		}

		lists = malloc(sizeof(List *)*(numargs-offset));
		arglens = malloc(sizeof(int)*(numargs-offset));

		/* convert ' ' to '_' */
		for(i=offset;i<numargs;i++) {
			lists[i-offset] = makeList(free);
			sp = args[i];
			while((sp = strchr(sp,' '))) *sp = '_';
			arglens[i-offset] = strlen(args[i]);
		}

		/* get list of songs to add */
		mpd_sendListallCommand(conn,"");
		printErrorAndExit(conn);
		while((entity = mpd_getNextInfoEntity(conn))) {
			if(entity->type==MPD_INFO_ENTITY_TYPE_SONG) {
				song = entity->info.song;
				sp = dup = strdup(fromUtf8(song->file));
				while((sp = strchr(sp,' '))) *sp = '_';
				len = strlen(dup);
				for(i=offset;i<numargs;i++) {
					if(len<arglens[i-offset]) continue;
					ret = strncmp(args[i],dup,
						arglens[i-offset]);
					if(ret==0) {
						insertInListWithoutKey(
							lists[i-offset],
							strdup(fromUtf8(
								song->file)));
					}
				}
				free(dup);
			}
			mpd_freeInfoEntity(entity);
		}
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		/* send list of songs */
		mpd_sendCommandListBegin(conn);
		printErrorAndExit(conn);
		for(i=offset;i<numargs;i++) {
			node = lists[i-offset]->firstNode;
			while(node) {
				printf("adding: %s\n",(char *)node->data);
				mpd_sendAddCommand(conn,toUtf8(node->data));
				printErrorAndExit(conn);
				node = node->nextNode;
			}
			freeList(lists[i-offset]);
		}
		mpd_sendCommandListEnd(conn);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		/* clean up */
		free(lists);
		if(argc==2 || (argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0)) {
			for(i=0;i<numargs;i++) free(args[i]);
			free(args);
		}
	}
	else if(strcmp(argv[1],"play")==0) {
		int song;
		int offset;
		int numargs;
		char ** args;
		int i;

		if(argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0) {
			offset = 0;
			numargs = stdinToArgArray(&args);
		}
		else if(argc==2 || argc==3) {
			offset = 2;
			numargs = argc;
			args = argv;
		}
		else {
			fprintf(stderr,"usage: %s play <song number/->\n",
				argv[0]);
			exit(-1);
		}

		if(argc==2) song = MPD_PLAY_AT_BEGINNING;
		else {
			char * s;
			char * t;

			for(i=offset;i<numargs-1;i++) {
				printf("skipping: %s\n",args[i]);
			}

			if(args[i][0]=='#') s = &(args[i][1]);
			else s = args[i];

			song = strtol(s,&t,10);
			if(s==t || (*t!=')' && *t!='\0')) {
				fprintf(stderr,"error parsing song numbers from"
					": %s\n",args[i]);
				exit(-1);
			}

			if(song<1) {
				fprintf(stderr,"\"%s\" is not a positive"
					"integer\n",args[i]);
				exit(-1);
			}
			song--;
		}

		if(argc==3 && strcmp(argv[2],STDIN_SYMBOL)==0) {
			for(i=0;i<numargs;i++) free(args[i]);
			free(args);
		}

		mpd_sendPlayCommand(conn,song);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		goto status;
	}
	else if(strcmp(argv[1],"move")==0) {
		int from;
		int to;

		if(argc!=4) {
			fprintf(stderr,"usage: %s move <from> <to>\n",argv[0]);
			return -1;
		}
		else {
			char * test;

			from = strtol(argv[2],&test,10);
			if(*test!='\0' || from<=0) {
				fprintf(stderr,"\"%s\" is not a positive "
						"integer\n",argv[2]);
				return -1;
			}

			to = strtol(argv[3],&test,10);
			if(*test!='\0' || to<=0) {
				fprintf(stderr,"\"%s\" is not a positive "
						"integer\n",argv[3]);
				return -1;
			}

			from--;
			to--;
		}

		mpd_sendMoveCommand(conn,from,to);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);
	}
	else if(strcmp(argv[1],"stop")==0) {
		if(argc!=2) {
			fprintf(stderr,"usage: %s stop\n",argv[0]);
			return -1;
		}

		mpd_sendStopCommand(conn);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		goto status;
	}
	else if(strcmp(argv[1],"pause")==0) {
		if(argc!=2) {
			fprintf(stderr,"usage: %s pause\n",argv[0]);
			return -1;
		}

		mpd_sendPauseCommand(conn);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		goto status;
	}
	else if(strcmp(argv[1],"prev")==0) {
		if(argc!=2) {
			fprintf(stderr,"usage: %s prev\n",argv[0]);
			return -1;
		}

		mpd_sendPrevCommand(conn);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		goto status;
	}
	else if(strcmp(argv[1],"next")==0) {
		if(argc!=2) {
			fprintf(stderr,"usage: %s next\n",argv[0]);
			return -1;
		}

		mpd_sendNextCommand(conn);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		goto status;
	}
	else if(strcmp(argv[1],"update")==0) {
		if(argc!=2) {
			fprintf(stderr,"usage: %s Update\n",argv[0]);
			return -1;
		}

		mpd_sendUpdateCommand(conn);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);
	}
	else if(strcmp(argv[1],"clear")==0) {
		if(argc!=2) {
			fprintf(stderr,"usage: %s clear\n",argv[0]);
			return -1;
		}

		mpd_sendClearCommand(conn);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);
		
		goto status;
	}
	else if(strcmp(argv[1],"shuffle")==0) {
		if(argc!=2) {
			fprintf(stderr,"usage: %s shuffle\n",argv[0]);
			return -1;
		}

		mpd_sendShuffleCommand(conn);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		goto status;
	}
	else if(strcmp(argv[1],"repeat")==0) {
		int mode;

		if(argc<2 || argc>3) {
			fprintf(stderr,"usage: %s repeat <on or off>\n",argv[0]);
			return -1;
		}

		if(argc==3) {
			if(strcmp(argv[2],"on")==0) mode = 1;
			else if(strcmp(argv[2],"off")==0) mode = 0;
			else {
				fprintf(stderr,"\"%s\" is not \"on\" or "
						"\"off\"\n",argv[2]);
				return -1;
			}
		}
		else {
			mpd_Status * status;
			status = mpd_getStatus(conn);
			printErrorAndExit(conn);
			mpd_finishCommand(conn);
			printErrorAndExit(conn);
			mode = !status->repeat;
			mpd_freeStatus(status);
		}


		mpd_sendRepeatCommand(conn,mode);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		goto status;
	}
	else if(strcmp(argv[1],"seek")==0) {
		mpd_Status * status;
		float perc;

		if(argc!=3) {
			fprintf(stderr,"usage: %s seek <0-100>\n",argv[0]);
			return -1;
		}
		else {
			char * test;

			perc = strtod(argv[2],&test);
			if(*test!='\0' || perc<0 || perc>100) {
				fprintf(stderr,"\"%s\" is not an number between"
						" 0 and 100\n",argv[2]);
				return -1;
			}
		}

		status = mpd_getStatus(conn);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		if(status->state==MPD_STATUS_STATE_STOP) {
			fprintf(stderr,"not currently playing\n");
			return -1;
		}

		mpd_sendSeekCommand(conn,status->song,
				perc*status->totalTime/100+0.5);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		mpd_freeStatus(status);

		goto status;
	}
	else if(strcmp(argv[1],"crossfade")==0) {
		int seconds;

		if(argc<2 || argc>3) {
			fprintf(stderr,"usage: %s crossfade <seconds>\n",argv[0]);
			return -1;
		}

		if(argc==3) {
			char * test;
			seconds = strtol(argv[2],&test,10);

			if(*test!='\0' || seconds<0) {
				fprintf(stderr,"\"%s\" is not 0 or positive integer\n",argv[2]);
			}

			mpd_sendCrossfadeCommand(conn,seconds);
			printErrorAndExit(conn);
			mpd_finishCommand(conn);
			printErrorAndExit(conn);
		}
		else {
			seconds = mpd_getCrossfade(conn);
			printErrorAndExit(conn);
			mpd_finishCommand(conn);
			printErrorAndExit(conn);

			printf("crossfade: %i\n",seconds);
		}
	}
	else if(strcmp(argv[1],"random")==0) {
		int mode;

		if(argc<2 || argc>3) {
			fprintf(stderr,"usage: %s random <on or off>\n",argv[0]);
			return -1;
		}

		if(argc==3) {
			if(strcmp(argv[2],"on")==0) mode = 1;
			else if(strcmp(argv[2],"off")==0) mode = 0;
			else {
				fprintf(stderr,"\"%s\" is not \"on\" or "
						"\"off\"\n",argv[2]);
				return -1;
			}
		}
		else {
			mpd_Status * status;
			status = mpd_getStatus(conn);
			printErrorAndExit(conn);
			mpd_finishCommand(conn);
			printErrorAndExit(conn);
			mode = !status->random;
			mpd_freeStatus(status);
		}

		mpd_sendRandomCommand(conn,mode);
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		goto status;
	}
	else if(strcmp(argv[1],"volume")==0) {
		int vol;
		int rel = 0;

		if(argc==3) {
			char * test;
			
			if (0 == strncmp(argv[2],"+",1))
				rel = 1;
			else if (0 == strncmp(argv[2],"-",1))
				rel = -1;

			vol = strtol(argv[2],&test,10);
			
			if (!rel && (*test!='\0' || vol<0)) {
				fprintf(stderr,"\"%s\" is not a positive "
						"integer\n",argv[2]);
				return -1;
			} else if (rel && (*test!='\0')) {
				fprintf(stderr,"\"%s\" is not an integer\n",
						argv[2]);
				return -1;
			}
		}
		else {
			mpd_Status *status = mpd_getStatus(conn);
			printErrorAndExit(conn);
			mpd_finishCommand(conn);
			printErrorAndExit(conn);

			printf("volume:%3i%c   \n",status->volume,'%');
			fprintf(stderr,"usage: %s volume [+-]<volume>\n",
					argv[0]);
			
			mpd_freeStatus(status);
			
			return -1;
		}
		
		if (rel)
			mpd_sendVolumeCommand(conn,vol);
		else 
			mpd_sendSetvolCommand(conn,vol);
		
		printErrorAndExit(conn);
		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		goto status;
	}
	else if(strcmp(argv[1],"version")==0) {
		printf("mpd version: %i.%i.%i\n",conn->version[0],
				conn->version[1],conn->version[2]);
	}
	else {
		fprintf(stderr,"unknown command \"%s\"\n",argv[1]);
		fprintf(stderr,"Usage: %s <command> [command args]...\n"
                        "mpc version: "VERSION"\n"
			"mpc                  Displays status\n"
			"mpc add <filename>   Add a song to the current playlist\n"
			"mpc del <playlist #> Remove a song from the current playlist \n"
			"mpc play <number>    Start playing at <number> (default: 1)\n"
			"mpc next             Play the next song in the current playlist\n"
			"mpc prev             Play the previous song in the current playlist\n"
			"mpc pause            Pauses the currently playing song\n"
			"mpc stop             Stop the currently playing playlist\n"
			"mpc seek <value>         Seeks to the position specified in percent (0-100)\n"
			"mpc clear            Clear the current playlist\n"
			"mpc shuffle          Shuffle the current playlist\n"
			"mpc move <from> <to> Move song in playlist\n"
			"mpc playlist         Print the current playlist\n"
			"mpc listall [<song>] Lists <song> from the current playlist.\n\t\t\tIf no song is specified, list all songs.\n"
                        "mpc ls [<dir>]       List the contents of <dir>\n"
			"mpc lsplaylists      Lists currently available playlists\n"
			"mpc load <file>      Load <file> as a playlist\n"
			"mpc save <file>      Saves a playlist as <file>\n"
			"mpc rm <file>        Removes a playlist\n"
			"mpc volume [+-]<num> Sets the volume to [+-]<num> (0-100) or (-100 to +100)\n"
			"mpc repeat <on|off>  Toggle repeat mode, or specify state\n"
			"mpc random <on|off>  Toggle random mode, or specify state\n"
			"mpc crossfade [sec]  Set and display crossfade settings\n"
			"mpc update           Scans music directory for updates\n"
			"mpc version          Reports version of MPD\n"
			"For more information about these and other options look man 1 mpc\n"
			,argv[0]);
		
	}

	mpd_closeConnection(conn);

	return 0;

status:
	/* print status */
	{
		mpd_Status * status;
		mpd_InfoEntity * entity;
		status = mpd_getStatus(conn);
		printErrorAndExit(conn);

		if(status->state == MPD_STATUS_STATE_PLAY || 
				status->state == MPD_STATUS_STATE_PAUSE) {
			mpd_sendPlaylistInfoCommand(conn,status->song);
			printErrorAndExit(conn);

			while((entity = mpd_getNextInfoEntity(conn))) {
				mpd_Song * song = entity->info.song;
				
				if(entity->type!=MPD_INFO_ENTITY_TYPE_SONG) {
					mpd_freeInfoEntity(entity);
					continue;
				}

				if(song->artist && song->title &&
					strlen(song->artist) &&
					strlen(song->title)) {
					printf("%s - ",fromUtf8(song->artist));
					printf("%s\n",fromUtf8(song->title));
				}
				else printf("%s\n",fromUtf8(song->file));

				mpd_freeInfoEntity(entity);

				break;
			}

			printErrorAndExit(conn);

			mpd_finishCommand(conn);
			printErrorAndExit(conn);

			if(status->state==MPD_STATUS_STATE_PLAY) {
				printf("[playing]");
			}
			else printf("[paused] ");

			printf(" #%i/%i %3i:%02i (%.0f%c)\n",
					status->song+1,
					status->playlistLength,
					status->elapsedTime/60,
					status->elapsedTime%60,
					100.0*status->elapsedTime/
					status->totalTime,'%');
		}

		if(status->volume!=MPD_STATUS_NO_VOLUME) {
			printf("volume:%3i%c   ",status->volume,'%');
		}
		else {
			printf("volume: n/a   ");
		}

		printf("repeat: ");
		if(status->repeat) printf("on    ");
		else printf("off   ");

		printf("random: ");
		if(status->random) printf("on \n");
		else printf("off\n");

		mpd_freeStatus(status);
	}

	mpd_closeConnection(conn);
	fclose(stdout);

	return 0;
}

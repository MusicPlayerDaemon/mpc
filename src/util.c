#include "util.h"
#include "libmpdclient.h"
#include "charConv.h"
#include "list.h"
#include "mpc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

void printErrorAndExit(mpd_Connection * conn)
{
	if(conn->error) {
		fprintf(stderr,"error: %s\n",fromUtf8(conn->errorStr));
		exit(-1);
	}
}

int stdinToArgArray(char *** array)
{
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

void free_pipe_array (int max, char ** array)
{
	int i;
	for ( i=0 ;i<max; ++i) free(array[i]);
	free(array);
}

int get_boolean (char * arg)
{
	int i;
	struct _bool_table {
		const char * on;
		const char * off;
	} bool_table [] = {
		{ "on", "off" },
		{ "1", "0" },
		{ "true", "false" },
		{ "yes", "no" },
		{}
	};

	for (i=0; bool_table[i].on; ++i) {
		if (! strcasecmp(arg,bool_table[i].on))
			return 1;
		else if (! strcasecmp(arg,bool_table[i].off))
			return 0;
	}
	
	fprintf(stderr,"\"%s\" is not boolean value: <",arg);
	
	for (i=0; bool_table[i].on; ++i) {
		fprintf(stderr,"%s|%s%s", bool_table[i].on,
			bool_table[i].off,
			( bool_table[i+1].off ? "|" : ">\n"));
	}  
	return -1;
}

#ifndef MPC_UTIL_H
#define MPC_UTIL_H

#include "libmpdclient.h"

void printErrorAndExit(mpd_Connection * conn);
void free_pipe_array (int max, char ** array);
int stdinToArgArray(char *** array);
int get_boolean (char * arg);

#endif /* MPC_UTIL_H */	

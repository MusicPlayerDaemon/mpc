#include "password.h"
#include "libmpdclient.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>

void parse_password ( const char * host,
		int * password_len,
		int * parsed_len )
{
	/* parse password and host */
	char * ret = strstr(host,"@");
	int len = ret-host;

	if(ret && len == 0) parsed_len++;
	else if(ret) {
		* password_len = len;
		* parsed_len += len+1;
	}
}

void send_password ( const char * host,
		int password_len,
		mpd_Connection * conn)
{
	char * dup = strdup(host);
	dup[password_len] = '\0';

	mpd_sendPasswordCommand(conn,dup);
	printErrorAndExit(conn);
	mpd_finishCommand(conn);
	printErrorAndExit(conn);

	free(dup);
}


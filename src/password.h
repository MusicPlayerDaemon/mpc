#ifndef PASSWORD_H
#define PASSWORD_H
#include "libmpdclient.h"
void parse_password (const char * host, int * pass_len, int * parsed_len);
void send_password (const char * host, int pass_len, mpd_Connection * conn);
#endif /* PASSWORD_H */

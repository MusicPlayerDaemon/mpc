#ifndef MPC_H
#define MPC_H

#include "libmpdclient.h"
#define STRING_LENGTH	(2*MAXPATHLEN)

#define STDIN_SYMBOL	"-"

#ifndef DEFAULT_HOST
#  define DEFAULT_HOST "localhost"
#endif /* DEFAULT_HOST */

#ifndef DEFAULT_PORT
#  define DEFAULT_PORT "2100"
#endif /* DEFAULT_PORT */

#ifndef MIN_COLUMNS
#  define MIN_COLUMNS	80
#endif /* MIN_COLUMNS */

typedef int (* cmdhandler)(int argc, char ** argv, mpd_Connection * conn);

#ifdef DEBUG
#  define dbg(fmt, arg...) do { printf("%s: "fmt,__func__, ## arg); } while (0)
#else
#  define dbg(fmt, arg...) do { } while (0)
#endif /* DEBUG */

#endif /* MPC_H */

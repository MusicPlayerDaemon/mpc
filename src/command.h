#ifndef COMMAND_H
#define COMMAND_H
#include "libmpdclient.h"
int cmd_status ( int argc, char ** argv, mpd_Connection *conn );
int cmd_add ( int argc, char ** argv, mpd_Connection * conn ) ;
int cmd_del ( int argc, char ** argv, mpd_Connection * conn );
int cmd_play ( int argc, char ** argv, mpd_Connection * conn );
int cmd_next ( int argc, char ** argv, mpd_Connection * conn );
int cmd_prev ( int argc, char ** argv, mpd_Connection * conn );
int cmd_pause ( int argc, char ** argv, mpd_Connection * conn );
int cmd_stop ( int argc, char ** argv, mpd_Connection * conn );
int cmd_seek ( int argc, char ** argv, mpd_Connection * conn );
int cmd_clear ( int argc, char ** argv, mpd_Connection * conn ) ;
int cmd_shuffle ( int argc, char ** argv, mpd_Connection * conn );
int cmd_move ( int argc, char ** argv, mpd_Connection * conn );
int cmd_playlist ( int argc, char ** argv, mpd_Connection * conn );
int cmd_listall ( int argc, char ** argv, mpd_Connection * conn );
int cmd_ls ( int argc, char ** argv, mpd_Connection * conn );
int cmd_lsplaylists ( int argc, char ** argv, mpd_Connection * conn );
int cmd_load ( int argc, char ** argv, mpd_Connection * conn );
int cmd_search ( int argc, char ** argv, mpd_Connection * conn ) ;
int cmd_save ( int argc, char ** argv, mpd_Connection * conn ) ;
int cmd_rm ( int argc, char ** argv, mpd_Connection * conn );
int cmd_volume ( int argc, char ** argv, mpd_Connection * conn ) ;
int cmd_repeat ( int argc, char ** argv, mpd_Connection * conn );
int cmd_random ( int argc, char ** argv, mpd_Connection * conn );
int cmd_crossfade ( int argc, char ** argv, mpd_Connection * conn );
int cmd_update ( int argc, char ** argv, mpd_Connection * conn );
int cmd_version ( int argc, char ** argv, mpd_Connection * conn );
int cmd_loadtab ( int argc, char ** argv, mpd_Connection * conn );
int cmd_lstab ( int argc, char ** argv, mpd_Connection * conn );
int cmd_tab ( int argc, char ** argv, mpd_Connection * conn );
#endif /* COMMAND_H */

/* mpc
 * (c) 2003-2004 by normalperson and Warren Dukes (warren.dukes@gmail.com)
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

#ifndef COMMAND_H
#define COMMAND_H
#include "libmpdclient.h"
int cmd_status ( int argc, char ** argv, mpd_Connection *conn );
int cmd_add ( int argc, char ** argv, mpd_Connection * conn ) ;
int cmd_crop ( int argc, char ** argv, mpd_Connection * conn );
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
int cmd_enable( int argc, char ** argv, mpd_Connection * conn );
int cmd_disable( int argc, char ** argv, mpd_Connection * conn );
int cmd_outputs ( int argc, char ** argv, mpd_Connection * conn );
int cmd_update ( int argc, char ** argv, mpd_Connection * conn );
int cmd_version ( int argc, char ** argv, mpd_Connection * conn );
int cmd_loadtab ( int argc, char ** argv, mpd_Connection * conn );
int cmd_lstab ( int argc, char ** argv, mpd_Connection * conn );
int cmd_tab ( int argc, char ** argv, mpd_Connection * conn );
int cmd_stats ( int argc, char ** argv, mpd_Connection *conn );
int cmd_toggle ( int argc, char ** argv, mpd_Connection *conn );
#endif /* COMMAND_H */

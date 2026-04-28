// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef COMMAND_H
#define COMMAND_H

struct mpd_connection;

int cmd_status(int argc, char **argv, struct mpd_connection *conn);
int cmd_current(int argc, char **argv, struct mpd_connection *conn);
int cmd_queued(int argc, char **argv, struct mpd_connection *conn);
int cmd_play(int argc, char **argv, struct mpd_connection *conn);
int cmd_searchplay(int argc, char **argv, struct mpd_connection *conn);
int cmd_next(int argc, char **argv, struct mpd_connection *conn);
int cmd_prev(int argc, char **argv, struct mpd_connection *conn);
int cmd_pause(int argc, char **argv, struct mpd_connection *conn);
int cmd_pause_if_playing(int argc, char **argv, struct mpd_connection *conn);
int cmd_stop(int argc, char **argv, struct mpd_connection *conn);
int cmd_seek(int argc, char **argv, struct mpd_connection *conn);
int cmd_seek_through(int argc, char **argv, struct mpd_connection *conn);
int cmd_clearerror(int argc, char **argv, struct mpd_connection *conn);
int cmd_move(int argc, char **argv, struct mpd_connection *conn);
int cmd_moveplaylist(int argc, char **argv, struct mpd_connection *conn);
int cmd_listall(int argc, char **argv, struct mpd_connection *conn);
int cmd_ls(int argc, char **argv, struct mpd_connection *conn);
int cmd_lsplaylists(int argc, char **argv, struct mpd_connection *conn);
int cmd_lsdirs(int argc, char **argv, struct mpd_connection *conn);
int cmd_addplaylist(int argc, char **argv, struct mpd_connection *conn);
int cmd_delplaylist(int argc, char **argv, struct mpd_connection *conn);
int cmd_renplaylist(int argc, char **argv, struct mpd_connection *conn);
int cmd_clearplaylist(int argc, char **argv, struct mpd_connection *conn);
int cmd_load(int argc, char **argv, struct mpd_connection *conn);
int cmd_list(int argc, char **argv, struct mpd_connection *conn);
int cmd_save(int argc, char **argv, struct mpd_connection *conn);
int cmd_rm(int argc, char **argv, struct mpd_connection *conn);
int cmd_volume(int argc, char **argv, struct mpd_connection *conn);
int cmd_repeat(int argc, char **argv, struct mpd_connection *conn);
int cmd_random(int argc, char **argv, struct mpd_connection *conn);
int cmd_single(int argc, char **argv, struct mpd_connection *conn);
int cmd_consume(int argc, char **argv, struct mpd_connection *conn);
int cmd_crossfade(int argc, char **argv, struct mpd_connection *conn);
int cmd_mixrampdb(int argc, char **argv, struct mpd_connection *conn);
int cmd_mixrampdelay(int argc, char **argv, struct mpd_connection *conn);
int cmd_update(int argc, char **argv, struct mpd_connection *conn);
int cmd_rescan(int argc, char **argv, struct mpd_connection *conn);
int cmd_version(int argc, char **argv, struct mpd_connection *conn);
int cmd_stats(int argc, char **argv, struct mpd_connection *conn);
int cmd_cdprev(int argc, char **argv, struct mpd_connection *conn);
int cmd_toggle(int argc, char **argv, struct mpd_connection *conn);
int cmd_partitionlist(int argc, char **argv, struct mpd_connection *conn);
int cmd_partitionmake(int argc, char **argv, struct mpd_connection *conn);
int cmd_partitiondelete(int argc, char **argv, struct mpd_connection *conn);
int cmd_replaygain(int argc, char **argv, struct mpd_connection *conn);
int cmd_channels(int argc, char **argv, struct mpd_connection *conn);
int cmd_sendmessage(int argc, char **argv, struct mpd_connection *conn);
int cmd_waitmessage(int argc, char **argv, struct mpd_connection *conn);
int cmd_subscribe(int argc, char **argv, struct mpd_connection *conn);
int cmd_tags(int argc, char **argv, struct mpd_connection *conn);
int cmd_playlistlength(int argc, char **argv, struct mpd_connection *conn);

#endif /* COMMAND_H */

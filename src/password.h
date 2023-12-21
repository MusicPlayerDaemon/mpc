// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef PASSWORD_H
#define PASSWORD_H

struct mpd_connection;

void
send_password(const char *password, struct mpd_connection *conn);

#endif /* PASSWORD_H */

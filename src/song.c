/* libmpdclient
   (c) 2003-2008 The Music Player Daemon Project
   This project's homepage is: http://www.musicpd.org

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   - Neither the name of the Music Player Daemon nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "song.h"
#include "str_pool.h"

#include <stdlib.h>

static void mpd_initSong(struct mpd_song *song) {
	song->file = NULL;
	song->artist = NULL;
	song->album = NULL;
	song->track = NULL;
	song->title = NULL;
	song->name = NULL;
	song->date = NULL;
	/* added by Qball */
	song->genre = NULL;
	song->composer = NULL;
	song->performer = NULL;
	song->disc = NULL;
	song->comment = NULL;

	song->time = MPD_SONG_NO_TIME;
	song->pos = MPD_SONG_NO_NUM;
	song->id = MPD_SONG_NO_ID;
}

static void mpd_finishSong(struct mpd_song *song) {
	if (song->file)
		str_pool_put(song->file);
	if (song->artist)
		str_pool_put(song->artist);
	if (song->album)
		str_pool_put(song->album);
	if (song->title)
		str_pool_put(song->title);
	if (song->track)
		str_pool_put(song->track);
	if (song->name)
		str_pool_put(song->name);
	if (song->date)
		str_pool_put(song->date);
	if (song->genre)
		str_pool_put(song->genre);
	if (song->composer)
		str_pool_put(song->composer);
	if (song->performer)
		str_pool_put(song->performer);
	if (song->disc)
		str_pool_put(song->disc);
	if (song->comment)
		str_pool_put(song->comment);
}

struct mpd_song *mpd_newSong(void) {
	struct mpd_song *ret = malloc(sizeof(*ret));

	mpd_initSong(ret);

	return ret;
}

void mpd_freeSong(struct mpd_song *song) {
	mpd_finishSong(song);
	free(song);
}

struct mpd_song *
mpd_songDup(const struct mpd_song *song)
{
	struct mpd_song *ret = mpd_newSong();

	if (song->file)
		ret->file = str_pool_dup(song->file);
	if (song->artist)
		ret->artist = str_pool_dup(song->artist);
	if (song->album)
		ret->album = str_pool_dup(song->album);
	if (song->title)
		ret->title = str_pool_dup(song->title);
	if (song->track)
		ret->track = str_pool_dup(song->track);
	if (song->name)
		ret->name = str_pool_dup(song->name);
	if (song->date)
		ret->date = str_pool_dup(song->date);
	if (song->genre)
		ret->genre = str_pool_dup(song->genre);
	if (song->composer)
		ret->composer = str_pool_dup(song->composer);
	if (song->performer)
		ret->performer = str_pool_dup(song->performer);
	if (song->disc)
		ret->disc = str_pool_dup(song->disc);
	if (song->comment)
		ret->comment = str_pool_dup(song->comment);

	ret->time = song->time;
	ret->pos = song->pos;
	ret->id = song->id;

	return ret;
}

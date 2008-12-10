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

#ifndef SONG_H
#define SONG_H

#define MPD_SONG_NO_TIME	-1
#define MPD_SONG_NO_NUM		-1
#define MPD_SONG_NO_ID		-1

/* mpd_Song
 * for storing song info returned by mpd
 */
struct mpd_song {
	/* filename of song */
	char * file;
	/* artist, maybe NULL if there is no tag */
	char * artist;
	/* title, maybe NULL if there is no tag */
	char * title;
	/* album, maybe NULL if there is no tag */
	char * album;
	/* track, maybe NULL if there is no tag */
	char * track;
	/* name, maybe NULL if there is no tag; it's the name of the current
	 * song, f.e. the icyName of the stream */
	char * name;
	/* date */
	char *date;

	/* added by qball */
	/* Genre */
	char *genre;
	/* Composer */
	char *composer;
	/* Performer */
	char *performer;
	/* Disc */
	char *disc;
	/* Comment */
	char *comment;

	/* length of song in seconds, check that it is not MPD_SONG_NO_TIME  */
	int time;
	/* if plchanges/playlistinfo/playlistid used, is the position of the
	 * song in the playlist */
	int pos;
	/* song id for a song in the playlist */
	int id;
};

/* mpd_newSong
 * use to allocate memory for a new mpd_Song
 * file, artist, etc all initialized to NULL
 * if your going to assign values to file, artist, etc
 * be sure to malloc or strdup the memory
 * use mpd_freeSong to free the memory for the mpd_Song, it will also
 * free memory for file, artist, etc, so don't do it yourself
 */
struct mpd_song *mpd_newSong(void);

/* mpd_freeSong
 * use to free memory allocated by mpd_newSong
 * also it will free memory pointed to by file, artist, etc, so be careful
 */
void mpd_freeSong(struct mpd_song *song);

/* mpd_songDup
 * works like strDup, but for a mpd_Song
 */
struct mpd_song *
mpd_songDup(const struct mpd_song *song);

#endif

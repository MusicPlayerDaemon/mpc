// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "audio_format.h"

#include <mpd/client.h>

#include <assert.h>
#include <stdio.h>

void
format_audio_format(char *buffer, size_t buffer_size,
		    const struct mpd_audio_format *audio_format)
{
	assert(buffer != NULL);
	assert(buffer_size > 0);
	assert(audio_format != NULL);

	if (audio_format->bits == MPD_SAMPLE_FORMAT_FLOAT)
		snprintf(buffer, buffer_size, "%u:f:%u", audio_format->sample_rate, audio_format->channels);
	else if (audio_format->bits == MPD_SAMPLE_FORMAT_DSD)
		snprintf(buffer, buffer_size, "%u:dsd:%u", audio_format->sample_rate, audio_format->channels);
	else
		snprintf(buffer, buffer_size, "%u:%u:%u", audio_format->sample_rate, audio_format->bits, audio_format->channels);
}

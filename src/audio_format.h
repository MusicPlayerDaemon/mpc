// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#pragma once

#include <stddef.h>

struct mpd_audio_format;

void
format_audio_format(char *buffer, size_t buffer_size,
		    const struct mpd_audio_format *audio_format);

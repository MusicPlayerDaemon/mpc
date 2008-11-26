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

#include "str_pool.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define NUM_SLOTS 4096

struct slot {
	struct slot *next;
	unsigned char ref;
	char value[1];
} __attribute__((packed));

static struct slot *slots[NUM_SLOTS];

static inline unsigned
calc_hash(const char *p)
{
	unsigned hash = 5381;

	assert(p != NULL);

	while (*p != 0)
		hash = (hash << 5) + hash + *p++;

	return hash;
}

static inline struct slot *
value_to_slot(char *value)
{
	return (struct slot*)(value - offsetof(struct slot, value));
}

static struct slot *slot_alloc(struct slot *next, const char *value)
{
	size_t length = strlen(value);
	struct slot *slot = malloc(sizeof(*slot) + length);
	if (slot == NULL)
		abort(); /* XXX */

	slot->next = next;
	slot->ref = 1;
	memcpy(slot->value, value, length + 1);
	return slot;
}

char *str_pool_get(const char *value)
{
	struct slot **slot_p, *slot;

	slot_p = &slots[calc_hash(value) % NUM_SLOTS];
	for (slot = *slot_p; slot != NULL; slot = slot->next) {
		if (strcmp(value, slot->value) == 0 && slot->ref < 0xff) {
			assert(slot->ref > 0);
			++slot->ref;
			return slot->value;
		}
	}

	slot = slot_alloc(*slot_p, value);
	*slot_p = slot;
	return slot->value;
}

char *str_pool_dup(char *value)
{
	struct slot *slot = value_to_slot(value);

	assert(slot->ref > 0);

	if (slot->ref < 0xff) {
		++slot->ref;
		return value;
	} else {
		/* the reference counter overflows above 0xff;
		   duplicate the value, and start with 1 */
		struct slot **slot_p =
			&slots[calc_hash(slot->value) % NUM_SLOTS];
		slot = slot_alloc(*slot_p, slot->value);
		*slot_p = slot;
		return slot->value;
	}
}

void str_pool_put(char *value)
{
	struct slot **slot_p, *slot;

	slot = value_to_slot(value);
	assert(slot->ref > 0);
	--slot->ref;

	if (slot->ref > 0)
		return;

	for (slot_p = &slots[calc_hash(value) % NUM_SLOTS];
	     *slot_p != slot;
	     slot_p = &(*slot_p)->next) {
		assert(*slot_p != NULL);
	}

	*slot_p = slot->next;
	free(slot);
}

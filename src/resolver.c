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

#include "resolver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#  include <ws2tcpip.h>
#  include <winsock.h>
#else
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <sys/socket.h>
#  include <sys/un.h>
#  include <netdb.h>
#endif

#ifndef MPD_NO_GAI
#  ifdef AI_ADDRCONFIG
#    define MPD_HAVE_GAI
#  endif
#endif

struct resolver {
	enum {
		TYPE_ZERO, TYPE_ONE, TYPE_ANY
	} type;

#ifdef MPD_HAVE_GAI
	struct addrinfo *ai;
	const struct addrinfo *next;
#else
	struct sockaddr_in sin;
#endif

	struct resolver_address current;

#ifndef WIN32
	struct sockaddr_un saun;
#endif
};

struct resolver *
resolver_new(const char *host, int port)
{
	struct resolver *resolver;

	resolver = malloc(sizeof(*resolver));
	if (resolver == NULL)
		return NULL;

#ifndef WIN32
	if (host[0] == '/') {
		size_t path_length = strlen(host);
		if (path_length >= sizeof(resolver->saun.sun_path)) {
			free(resolver);
			return NULL;
		}

		resolver->saun.sun_family = AF_UNIX;
		memcpy(resolver->saun.sun_path, host, path_length + 1);

		resolver->current.family = PF_UNIX;
		resolver->current.protocol = 0;
		resolver->current.addrlen = sizeof(resolver->saun);
		resolver->current.addr = (const struct sockaddr *)&resolver->saun;
		resolver->type = TYPE_ONE;
	} else
#endif
	{
#ifdef MPD_HAVE_GAI
		struct addrinfo hints;
		char service[20];
		int ret;

		memset(&hints, 0, sizeof(hints));
		hints.ai_flags = AI_ADDRCONFIG;
		hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		snprintf(service, sizeof(service), "%d", port);

		ret = getaddrinfo(host, service, &hints, &resolver->ai);
		if (ret != 0) {
			free(resolver);
			return NULL;
		}

		resolver->next = resolver->ai;
		resolver->type = TYPE_ANY;
#else
		const struct hostent *he;

		he = gethostbyname(host);
		if (he == NULL) {
			free(resolver);
			return NULL;
		}

		if (he->h_addrtype != AF_INET) {
			free(resolver);
			return NULL;
		}


		memset(&resolver->sin, 0, sizeof(resolver->sin));
		resolver->sin.sin_family = AF_INET;
		resolver->sin.sin_port = htons(port);
		memcpy((char *)&resolver->sin.sin_addr.s_addr,
		       (char *)he->h_addr, he->h_length);

		resolver->current.family = PF_INET;
		resolver->current.protocol = 0;
		resolver->current.addrlen = sizeof(resolver->sin);
		resolver->current.addr = (const struct sockaddr *)&resolver->sin;

		resolver->type = TYPE_ONE;
#endif
	}

	return resolver;
}

void
resolver_free(struct resolver *resolver)
{
#ifdef MPD_HAVE_GAI
	if (resolver->type == TYPE_ANY)
		freeaddrinfo(resolver->ai);
#endif
	free(resolver);
}

const struct resolver_address *
resolver_next(struct resolver *resolver)
{
	if (resolver->type == TYPE_ZERO)
		return NULL;

	if (resolver->type == TYPE_ONE) {
		resolver->type = TYPE_ZERO;
		return &resolver->current;
	}

#ifdef MPD_HAVE_GAI
	if (resolver->next == NULL)
		return NULL;

	resolver->current.family = resolver->next->ai_family;
	resolver->current.protocol = resolver->next->ai_protocol;
	resolver->current.addrlen = resolver->next->ai_addrlen;
	resolver->current.addr = resolver->next->ai_addr;

	resolver->next = resolver->next->ai_next;

	return &resolver->current;
#else
	return NULL;
#endif
}

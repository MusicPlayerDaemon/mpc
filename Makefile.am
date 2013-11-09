ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign 1.11 dist-xz

man_MANS = doc/mpc.1
doc_DATA = AUTHORS COPYING NEWS README \
	doc/mpd-m3u-handler.sh doc/mpd-pls-handler.sh doc/mppledit \
	doc/mpc-completion.bash
EXTRA_DIST = $(man_MANS) $(doc_DATA)

bin_PROGRAMS = src/mpc

src_mpc_SOURCES = \
	src/main.c src/mpc.h \
	src/list.c list.h \
	src/password.c src/password.h \
	src/status.c src/status.h \
	src/util.c src/util.h \
	src/command.c src/command.h \
	src/sticker.c src/sticker.h \
	src/idle.c src/idle.h \
	src/message.c src/message.h \
	src/search.c src/search.h \
	src/options.c src/options.h \
	src/path.c src/path.h \
	src/gcc.h

if HAVE_ICONV
src_mpc_SOURCES += src/charset.c src/charset.h
endif

src_mpc_CPPFLAGS = $(AM_CPPFLAGS) $(ICONV_CFLAGS) $(LIBMPDCLIENT_CFLAGS)
src_mpc_LDADD = $(ICONV_LIBS) $(LIBMPDCLIENT_LIBS)

# sparse is a semantic parser
# URL: git://www.kernel.org/pub/scm/devel/sparse/sparse.git
SPARSE = sparse
SPARSE_FLAGS =
SPARSE_CPPFLAGS = $(DEFAULT_INCLUDES) \
	-I$(shell $(CC) -print-file-name=include) \
	-I$(shell $(CC) -print-file-name=include-fixed)
sparse-check:
	for i in $(src_mpc_SOURCES); \
	do \
		$(SPARSE) -I. $(SPARSE_FLAGS) $(SPARSE_CPPFLAGS) \
			$(src_mpc_CFLAGS) $(src_mpc_CPPFLAGS) \
			$(srcdir)/$$i || exit; \
	done

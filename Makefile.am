ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign 1.11 dist-xz subdir-objects

man_MANS = doc/mpc.1
doc_DATA = AUTHORS COPYING NEWS README \
	doc/mpd-m3u-handler.sh doc/mpd-pls-handler.sh doc/mppledit \
	doc/mpc-completion.bash
EXTRA_DIST = $(man_MANS) $(doc_DATA)

bin_PROGRAMS = src/mpc

src_mpc_SOURCES = \
	src/main.c src/mpc.h \
	src/list.c src/list.h \
	src/password.c src/password.h \
	src/status.c src/status.h \
	src/args.c src/args.h \
	src/format.c src/format.h \
	src/song_format.c src/song_format.h \
	src/util.c src/util.h \
	src/command.c src/command.h \
	src/queue.c src/queue.h \
	src/sticker.c src/sticker.h \
	src/tab.c src/tab.h \
	src/idle.c src/idle.h \
	src/message.c src/message.h \
	src/search.c src/search.h \
	src/options.c src/options.h \
	src/path.c src/path.h \
	src/Compiler.h

if HAVE_ICONV
src_mpc_SOURCES += src/charset.c src/charset.h
endif

src_mpc_CPPFLAGS = $(AM_CPPFLAGS) $(ICONV_CFLAGS) $(LIBMPDCLIENT_CFLAGS)
src_mpc_LDADD = $(ICONV_LIBS) $(LIBMPDCLIENT_LIBS)

if ENABLE_TEST
C_TESTS = \
	test/test_format

TESTS = $(C_TESTS)

check_PROGRAMS = \
	$(C_TESTS)

test_test_format_SOURCES = \
	src/format.c \
	src/song_format.c \
	test/test_format.c
test_test_format_CFLAGS = $(AM_CFLAGS) $(CHECK_CFLAGS)
test_test_format_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/src $(ICONV_CFLAGS) $(LIBMPDCLIENT_CFLAGS)
test_test_format_LDADD = $(ICONV_LIBS) $(LIBMPDCLIENT_LIBS) $(CHECK_LIBS) -lrt -lm
test_test_format_LDFLAGS = -pthread

if HAVE_ICONV
test_test_format_SOURCES += src/charset.c src/charset.h
endif

endif

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

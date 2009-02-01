ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign 1.9 dist-bzip2

man_MANS = doc/mpc.1
doc_DATA = AUTHORS COPYING NEWS README \
	doc/mpd-m3u-handler.sh doc/mpd-pls-handler.sh doc/mppledit \
	doc/mpc-bashrc
EXTRA_DIST = $(man_MANS) $(doc_DATA)

bin_PROGRAMS = src/mpc

src_mpc_headers = src/libmpdclient.h src/str_pool.h \
	src/song.h src/resolver.h \
	src/list.h \
	src/password.h \
	src/charset.h \
	src/status.h \
	src/util.h \
	src/command.h \
	src/mpc.h \
	src/options.h \
	src/gcc.h

src_mpc_SOURCES = src/libmpdclient.c src/str_pool.c \
	src/song.c src/resolver.c \
	src/main.c \
	src/list.c \
	src/password.c \
	src/status.c \
	src/util.c \
	src/command.c \
	src/options.c \
	$(src_mpc_headers)

if HAVE_ICONV
src_mpc_SOURCES += src/charset.c
endif

src_mpc_CPPFLAGS = $(AM_CPPFLAGS) $(ICONV_CFLAGS)
src_mpc_LDADD = $(MPC_LIBS) $(ICONV_LIBS)

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

ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign 1.9 dist-bzip2

SUBDIRS = doc src
doc_DATA = AUTHORS COPYING NEWS README
EXTRA_DIST = $(doc_DATA)

sparse-check:
	$(MAKE) -C src $@

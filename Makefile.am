ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign 1.9 dist-bzip2

SUBDIRS = doc src
pkgdata_DATA =
docdir = $(prefix)/share/doc/$(PACKAGE)
doc_DATA = AUTHORS README
EXTRA_DIST = ChangeLog COPYING $(pkgdata_DATA) $(doc_DATA)

sparse-check:
	$(MAKE) -C src $@

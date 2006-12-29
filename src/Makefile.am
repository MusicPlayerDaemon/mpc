bin_PROGRAMS = mpc
# lots of += lines makes diffs easier to apply :)

mpc_headers  = libmpdclient.h 
mpc_headers += list.h
mpc_headers += password.h
mpc_headers += charConv.h
mpc_headers += status.h
mpc_headers += util.h
mpc_headers += command.h
mpc_headers += mpc.h
mpc_headers += mpc_compat.h
mpc_headers += options.h

mpc_SOURCES  = libmpdclient.c 
mpc_SOURCES += main.c
mpc_SOURCES += list.c 
mpc_SOURCES += password.c 
mpc_SOURCES += charConv.c
mpc_SOURCES += status.c
mpc_SOURCES += util.c
mpc_SOURCES += command.c
mpc_SOURCES += options.c
mpc_SOURCES += $(mpc_headers)

mpc_CFLAGS = $(MPC_CFLAGS)
mpc_LDADD = $(MPC_LIBS)

# sparse is a semantic parser
# URL: git://www.kernel.org/pub/scm/devel/sparse/sparse.git
SPARSE = sparse
SPARSE_FLAGS =
sparse-check:
	for i in $(mpc_SOURCES); \
	do \
		$(SPARSE) -I. $(mpc_CFLAGS) $(SPARSE_FLAGS) $(srcdir)/$$i || exit; \
	done


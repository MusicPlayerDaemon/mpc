dnl AC_INIT(src/main.c)
dnl AM_INIT_AUTOMAKE(mpc, 0.10.0)

AC_PREREQ(2.52)
AC_INIT(mpc, 0.10.1, shank@mercury.chem.pitt.edu)
AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

set -- $CFLAGS
CFLAGS="-Wall $CFLAGS"

AC_ARG_WITH(default-host,[  --with-default-host=HOST  Use as host when MPD_HOST environment variable is not set], DEFAULT_HOST="$withval",DEFAULT_HOST="localhost")
AC_ARG_WITH(default-port,[  --with-default-port=PORT  Use as port when MPD_PORT environment variable is not set], DEFAULT_PORT="$withval",DEFAULT_PORT="2100")

AC_CHECK_LIB(socket,socket,LIBS="$LIBS -lsocket",)
AC_CHECK_LIB(nsl,gethostbyname,LIBS="$LIBS -lnsl",)

AM_ICONV
CFLAGS="$CFLAGS $INCICONV"
CFLAGS="$CFLAGS $LIBICONV"

AC_CHECK_HEADER(langinfo.h,CFLAGS="$CFLAGS -DHAVE_LANGINFO",)
AC_CHECK_HEADER(locale.h,CFLAGS="$CFLAGS -DHAVE_LOCALE",)

CFLAGS="$CFLAGS -DDEFAULT_HOST=\\\"$DEFAULT_HOST\\\""
CFLAGS="$CFLAGS -DDEFAULT_PORT=\\\"$DEFAULT_PORT\\\""

AC_OUTPUT(doc/Makefile src/Makefile Makefile)

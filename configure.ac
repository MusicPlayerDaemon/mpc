dnl AC_INIT(src/main.c)
dnl AM_INIT_AUTOMAKE(mpc, 0.12.0)

AC_PREREQ(2.58)
AC_INIT(mpc, 0.12.0, warren.dukes@gmail.com)
AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)

AC_PROG_CC
AC_PROG_INSTALL
dnl AC_PROG_LIBTOOL

AM_CONFIG_HEADER(config.h)

set -- $CFLAGS

case "$CC" in
*gcc*)
	CFLAGS="-Wall"
	;;
esac

EXTRA_CFLAGS=
EXTRA_LDFLAGS=
EXTRA_LIBS=
case "$host" in
*-mingw32* | *-windows*)
	EXTRA_CFLAGS="-mno-cygwin -mms-bitfields -fno-strict-aliasing -DMPD_NO_GAI"
	EXTRA_LDFLAGS="-mno-cygwin -mms-bitfields -no-undefined"
	EXTRA_LIBS="-lws2_32"
	;;
*-cygwin*)
	EXTRA_CFLAGS="-mms-bitfields -fno-strict-aliasing -DMPD_NO_GAI"
	EXTRA_LDFLAGS="-mms-bitfields"
	;;
esac
AC_SUBST(EXTRA_CFLAGS)
AC_SUBST(EXTRA_LDFLAGS)
AC_SUBST(EXTRA_LIBS)

if test -z "$prefix" || test "x$prefix" = xNONE; then
	local_lib=
	local_include=

	# aren't autotools supposed to be smart enough to figure this out?
	# oh well, the git-core Makefile managed to do some of the work for us :)
	case "`uname -s | tr A-Z a-z`" in
	darwin*)
		local_lib='/sw/lib /opt/local/lib'
		local_include='/sw/include /opt/local/include'
		;;
	freebsd* | openbsd*)
		local_lib=/usr/local/lib
		local_include=/usr/local/include
		;;
	netbsd*)
		local_lib=/usr/pkg/lib
		local_include=/usr/pkg/include
		LDFLAGS="$LDFLAGS -Wl,-rpath,/usr/pkg/lib"
		;;
	esac

	for d in $local_lib; do
		if test -d "$d"; then
			LDFLAGS="$LDFLAGS -L$d"
			break
		fi
	done
	for d in $local_include; do
		if test -d "$d"; then
			CFLAGS="$CFLAGS -I$d"
			break
		fi
	done
fi

AC_ARG_ENABLE(iconv,[  --disable-iconv     disable iconv support],,enable_iconv=yes)

AC_ARG_WITH(default-host,[  --with-default-host=HOST  Use as host when MPD_HOST environment variable is not set], DEFAULT_HOST="$withval",DEFAULT_HOST="localhost")
AC_ARG_WITH(default-port,[  --with-default-port=PORT  Use as port when MPD_PORT environment variable is not set], DEFAULT_PORT="$withval",DEFAULT_PORT="6600")

AC_ARG_WITH(iconv,[  --with-iconv=PFX   Prefix where iconv is installed (optional)], iconv_prefix="$withval", iconv_prefix="")
AC_ARG_WITH(iconv-libraries,[  --with-iconv-libraries=DIR   Directory where libiconv library is installed (optional)], iconv_libraries="$withval", iconv_libraries="")
AC_ARG_WITH(iconv-includes,[  --with-iconv-includes=DIR   Directory where libiconv header files are installed (optional)], iconv_includes="$withval", iconv_includes="")

AC_CHECK_LIB(socket,socket,LIBS="$LIBS -lsocket",)
AC_CHECK_LIB(nsl,gethostbyname,LIBS="$LIBS -lnsl",)

if test x$enable_iconv = xyes; then
  	if test "x$iconv_libraries" != "x" ; then
  		ICONV_LIBS="-L$iconv_libraries"
  	elif test "x$iconv_prefix" != "x" ; then
    		ICONV_LIBS="-L$iconv_prefix/lib"
  	elif test "x$prefix" != "xNONE"; then
    		ICONV_LIBS="-L$libdir"
  	fi

  	ICONV_LIBS="$ICONV_LIBS -liconv"

  	if test "x$iconv_includes" != "x" ; then
  		ICONV_CFLAGS="-I$iconv_includes"
  	elif test "x$iconv_prefix" != "x" ; then
    		ICONV_CFLAGS="-I$iconv_prefix/include"
  	elif test "x$prefix" != "xNONE"; then
    		ICONV_CFLAGS="-I$includedir"
  	fi

	CFLAGS="$CFLAGS $ICONV_CFLAGS"
	LIBS="$LIBS $ICONV_LIBS"
	CPPFLAGS=$CFLAGS
	AC_CHECK_HEADER(iconv.h,,[CFLAGS=$oldcflags;enable_iconv=no])
	if test x$enable_iconv = xyes; then
		AC_CHECK_LIB(iconv, main,,LIBS=$oldlibs)
		AC_DEFINE(HAVE_ICONV,1,[Define to use iconv])
	fi
fi


AM_LANGINFO_CODESET

AC_CHECK_HEADER(locale.h,AC_DEFINE(HAVE_LOCALE,1,[Define for locale]),)

AC_DEFINE_UNQUOTED(DEFAULT_HOST,"$DEFAULT_HOST",[Define default host])
AC_DEFINE_UNQUOTED(DEFAULT_PORT,"$DEFAULT_PORT",[Define default port])

AC_OUTPUT(doc/Makefile src/Makefile Makefile)

AC_PREREQ(2.60)
AC_INIT(mpc, 0.15, musicpd-dev-team@lists.sourceforge.net)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(src/main.c)
AM_INIT_AUTOMAKE([foreign 1.9 dist-bzip2])
AM_CONFIG_HEADER(config.h)


dnl
dnl programs
dnl

AC_PROG_CC_C99
AC_PROG_INSTALL
AC_PROG_MAKE_SET


dnl
dnl declare variables
dnl

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CPPFLAGS)

AC_SUBST(MPC_LIBS)
MPC_LIBS=""


dnl
dnl OS specific defaults
dnl

case "$host" in
*-mingw32* | *-windows*)
	AM_CFLAGS="$AM_CFLAGS -mno-cygwin -mms-bitfields -fno-strict-aliasing"
	AM_CPPFLAGS="$AM_CPPFLAGS -DMPD_NO_GAI"
	MPC_LIBS="$MPC_LIBS -lws2_32"
	;;
*-cygwin*)
	AM_CFLAGS="$AM_CFLAGS -mms-bitfields -fno-strict-aliasing"
	AM_CPPFLAGS="$AM_CPPFLAGS -DMPD_NO_GAI"
	;;
esac

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


dnl
dnl libc features
dnl

AC_CHECK_LIB(socket, socket, MPC_LIBS="$MPC_LIBS -lsocket",)
AC_CHECK_LIB(nsl, gethostbyname, MPC_LIBS="$MPC_LIBS -lnsl",)


dnl
dnl i18n / l10n (iconv)
dnl

AC_ARG_ENABLE(iconv,
	AS_HELP_STRING([--disable-iconv],
		[disable iconv support (default: enable)]),,
	[enable_iconv=yes])

if test x$enable_iconv = xyes; then
	AC_CHECK_FUNC(iconv,
		[ICONV_CFLAGS="" ICONV_LIBS=""],
		AC_CHECK_LIB(intl, iconv,
			[ICONV_CFLAGS="" ICONV_LIBS="-lintl"],
			[enable_iconv=no]))
fi

if test x$enable_iconv = xyes; then
	AM_LANGINFO_CODESET
	if test x$am_cv_langinfo_codeset != xyes; then
		enable_iconv=no
		AC_MSG_WARN(nl_langinfo and CODESET not available - disabling iconv)
	fi
fi

if test x$enable_iconv = xyes; then
	AC_CHECK_HEADER([locale.h],, [enable_iconv=no])
	if test x$enable_iconv != xyes; then
		AC_MSG_WARN(locale.h not available - disabling iconv)
	fi
fi

if test x$enable_iconv = xyes; then
	AC_DEFINE(HAVE_ICONV, 1, [Define if iconv() support is enabled])
else
	ICONV_CPPFLAGS=""
	ICONV_LIBS=""
fi

AC_SUBST(ICONV_CPPFLAGS)
AC_SUBST(ICONV_LIBS)

AM_CONDITIONAL(HAVE_ICONV, test x$enable_iconv = xyes)


dnl
dnl MPD protocol features
dnl

AC_ARG_WITH(default-host,
	AS_HELP_STRING([--with-default-host=HOST],
		[Use as host when MPD_HOST environment variable is not set]),
	[DEFAULT_HOST="$withval"],
	[DEFAULT_HOST="localhost"])

AC_ARG_WITH(default-port,
	AS_HELP_STRING([--with-default-port=PORT],
		[Use as port when MPD_PORT environment variable is not set]),
	[DEFAULT_PORT="$withval"],
	[DEFAULT_PORT="6600"])

AC_DEFINE_UNQUOTED(DEFAULT_HOST,"$DEFAULT_HOST",[Define default host])
AC_DEFINE_UNQUOTED(DEFAULT_PORT,"$DEFAULT_PORT",[Define default port])


dnl
dnl CFLAGS
dnl

CHECK_CFLAG([-Wall])
CHECK_CFLAG([-Wextra])
CHECK_CFLAG([-Wno-deprecated-declarations])
CHECK_CFLAG([-Wmissing-prototypes])
CHECK_CFLAG([-Wdeclaration-after-statement])
CHECK_CFLAG([-Wshadow])
CHECK_CFLAG([-Wpointer-arith])
CHECK_CFLAG([-Wstrict-prototypes])
CHECK_CFLAG([-Wcast-qual])
CHECK_CFLAG([-Wwrite-strings])


dnl
dnl build options
dnl

AC_ARG_ENABLE(werror,
	AS_HELP_STRING([--enable-werror],
		[Treat warnings as errors (default: disabled)]),
	ENABLE_WERROR=$enableval,
	ENABLE_WERROR=no)

if test x$ENABLE_WERROR = xyes; then
	AM_CFLAGS="$AM_CFLAGS -Werror -pedantic-errors"
fi

AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug],
		[Enable debugging (default: disabled)]),
	ENABLE_DEBUG=$enableval,
	ENABLE_DEBUG=no)

if test x$ENABLE_DEBUG = xno; then
	AM_CPPFLAGS="$AM_CPPFLAGS -DNDEBUG"
fi


dnl

AC_OUTPUT(Makefile)

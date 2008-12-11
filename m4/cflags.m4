AC_DEFUN([CHECK_CFLAG], [
    var=`echo "$1" | tr "=-" "__"`
    AC_CACHE_CHECK([whether the C compiler accepts $1],[check_cflag_$var],[
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $1"
        AC_COMPILE_IFELSE([int main(void) { return 0; }],
            [eval "check_cflag_$var=yes"],
            [eval "check_cflag_$var=no"])
        CFLAGS="$save_CFLAGS"
        ])
    if eval "test x`echo '$check_cflag_'$var` = xyes"; then
        AM_CFLAGS="$AM_CFLAGS $1"
    fi
    ])
])

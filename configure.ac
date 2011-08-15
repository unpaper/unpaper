AC_PREREQ(2.68)

AC_INIT([unpaper], [0.4])

AC_PROG_CC_C99

AM_INIT_AUTOMAKE([dist-xz no-dist-gzip foreign])
AC_USE_SYSTEM_EXTENSIONS

AC_HEADER_STDBOOL
AC_SEARCH_LIBS([sqrt], [m])

AC_CHECK_FUNCS_ONCE([sincosf])

AS_IF([test "$crosscompiling" != "no"], [
  AC_PATH_PROG([PNGTOPNM], [pngtopnm], [false])
  AC_PATH_PROG([PNMTOPNG], [pnmtopng], [false])

  AM_CONDITIONAL([TESTS_ENABLED], [test "$PNGTOPNM" != "false" -a "$PNMTOPNG" != "false"])
])

AC_CONFIG_FILES([Makefile tests/prologue.sh])

AC_OUTPUT

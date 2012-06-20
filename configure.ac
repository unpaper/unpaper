AC_PREREQ(2.68)

AC_INIT([unpaper], [0.4.1])

AC_PROG_MKDIR_P
AC_PROG_CC_C99

AM_INIT_AUTOMAKE([dist-xz no-dist-gzip foreign parallel-tests])
AC_USE_SYSTEM_EXTENSIONS

AC_HEADER_STDBOOL
AC_SEARCH_LIBS([sqrt], [m])

AC_PATH_PROG([PNGTOPNM], [pngtopnm], [false])
AC_PATH_PROG([PNMTOPNG], [pnmtopng], [false])

AC_ARG_VAR([XSLTPROC], [A libxslt-compatible XSLT processor command])
AS_IF([test "x$XSLTPROC" = "x"], [
  AC_CHECK_PROGS([XSLTPROC], [xsltproc])
  AS_IF([test "x$XSLTPROC" = "x"], [
    AC_MSG_ERROR([unable to find xsltproc, you need this to build man pages])
  ])
])

AC_CONFIG_FILES([Makefile tests/prologue.sh])

AC_OUTPUT

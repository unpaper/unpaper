AC_PREREQ(2.68)

AC_INIT([unpaper], [6.1], [https://github.com/Flameeyes/unpaper/issues], , [https://github.com/Flameeyes/unpaper])

AM_INIT_AUTOMAKE([1.13 dist-xz no-dist-gzip foreign])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([enable])

AC_PROG_CC_C99
AC_PROG_MKDIR_P

AC_USE_SYSTEM_EXTENSIONS

AC_HEADER_STDBOOL
AC_SEARCH_LIBS([sqrt], [m])

PKG_CHECK_MODULES([LIBAV], [libavformat libavcodec libavutil])

AC_ARG_VAR([XSLTPROC], [A libxslt-compatible XSLT processor command])
AS_IF([test "x$XSLTPROC" = "x"], [
  AC_CHECK_PROGS([XSLTPROC], [xsltproc])
  AS_IF([test "x$XSLTPROC" = "x"], [
    AC_MSG_ERROR([unable to find xsltproc, you need this to build man pages])
  ])
])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

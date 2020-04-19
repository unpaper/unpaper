dnl SPDX-FileCopyrightText: 2005 The unpaper authors
dnl
dnl SPDX-License-Identifier: GPL-2.0-only

AC_PREREQ(2.68)

AC_INIT([unpaper], [6.2], [https://github.com/unpaper/unpaper/issues], , [https://github.com/unpaper/unpaper])

AM_INIT_AUTOMAKE([1.13 dist-xz no-dist-gzip foreign subdir-objects])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([enable])

AC_PROG_CC_C99
AC_PROG_MKDIR_P

AC_USE_SYSTEM_EXTENSIONS

AC_SEARCH_LIBS([sqrt], [m])

PKG_CHECK_MODULES([FFMPEG], [libavformat libavcodec libavutil])

AC_ARG_VAR([XSLTPROC], [A libxslt-compatible XSLT processor command])
AS_IF([test "x$XSLTPROC" = "x"], [
  AC_CHECK_PROGS([XSLTPROC], [xsltproc])
  AS_IF([test "x$XSLTPROC" = "x"], [
    AC_MSG_ERROR([unable to find xsltproc, you need this to build man pages])
  ])
])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

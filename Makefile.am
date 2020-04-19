# SPDX-FileCopyrightText: 2005 The unpaper authors
#
# SPDX-License-Identifier: GPL-2.0-only

bin_PROGRAMS = unpaper
MANPAGE_XSL ?= http://docbook.sourceforge.net/release/xsl-ns/current/manpages/docbook.xsl

unpaper_SOURCES = file.c imageprocess.c imageprocess.h parse.c  \
	parse.h tools.c tools.h unpaper.c unpaper.h
unpaper_CFLAGS = $(FFMPEG_CFLAGS)
unpaper_LDADD = $(FFMPEG_LIBS)

dist_doc_DATA = NEWS README.md AUTHORS LICENSES/CC0-1.0.txt     \
                LICENSES/GPL-2.0-only.txt
dist_man_MANS = unpaper.1

dist_html_DATA = doc/basic-concepts.md doc/image-processing.md  \
        doc/file-formats.md
imgdir = $(htmldir)/img
dist_img_DATA = doc/img/blackfilter-detail.png                          \
                doc/img/blackfilter-detail.png.license                  \
                doc/img/blackfilter.png                                 \
                doc/img/blackfilter.png.license                         \
                doc/img/blurfilter-detail.png                           \
                doc/img/blurfilter-detail.png.license                   \
                doc/img/blurfilter.png doc/img/blurfilter.png.license   \
                doc/img/border-align.png                                \
                doc/img/border-align.png.license                        \
                doc/img/border-scan.png                                 \
                doc/img/border-scan.png.license                         \
                doc/img/deskew-detail1.png                              \
                doc/img/deskew-detail1.png.license                      \
                doc/img/deskew-detail2.png                              \
                doc/img/deskew-detail2.png.license doc/img/deskew.png   \
                doc/img/deskew.png.license doc/img/documentation.odg    \
                doc/img/documentation.odg.license                       \
                doc/img/input-pages.png                                 \
                doc/img/input-pages.png.license                         \
                doc/img/layout-templates.png                            \
                doc/img/layout-templates.png.license                    \
                doc/img/mask-center.png                                 \
                doc/img/mask-center.png.license                         \
                doc/img/mask-scan-detail1.png                           \
                doc/img/mask-scan-detail1.png.license                   \
                doc/img/mask-scan-detail2.png                           \
                doc/img/mask-scan-detail2.png.license                   \
                doc/img/mask-scan.png doc/img/mask-scan.png.license     \
                doc/img/multiple-input-files.png                        \
                doc/img/multiple-input-files.png.license                \
                doc/img/multiple-output-files.png                       \
                doc/img/multiple-output-files.png.license               \
                doc/img/noisefilter.png                                 \
                doc/img/noisefilter.png.license                         \
                doc/img/output-pages.png                                \
                doc/img/output-pages.png.license                        \
                doc/img/processing-order.png                            \
                doc/img/processing-order.png.license                    \
                doc/img/sheetspages.png                                 \
                doc/img/sheetspages.png.license                         \
                doc/img/standard-deviation.png                          \
                doc/img/standard-deviation.png.license

check_PROGRAMS = compare-image
compare_image_SOURCES = tests/compare-image.c
compare_image_CFLAGS = $(FFMPEG_CFLAGS)
compare_image_LDADD = $(FFMPEG_LIBS)

TESTS = tests/runtestA1.sh tests/runtestB1.sh tests/runtestB2.sh	\
        tests/runtestB3.sh tests/runtestC1.sh tests/runtestC2.sh	\
        tests/runtestC3.sh tests/runtestD1.sh tests/runtestD2.sh	\
        tests/runtestD3.sh tests/runtestE1.sh tests/runtestE2.sh	\
        tests/runtestE3.sh tests/runtestF1.sh tests/runtestF2.sh	\
        tests/runtestF3.sh tests/runtestG1.sh tests/runtestG2.sh	\
	tests/runtestG3.sh

XFAIL_TESTS = tests/runtestG3.sh

EXTRA_DIST = $(TESTS) tests/goldenA1.pbm tests/goldenA1.pbm.license     \
             tests/goldenB1.ppm tests/goldenB1.ppm.license              \
             tests/goldenB2.ppm tests/goldenB2.ppm.license              \
             tests/goldenB3.ppm tests/goldenB3.ppm.license              \
             tests/goldenC1.pbm tests/goldenC1.pbm.license              \
             tests/goldenC2.pbm tests/goldenC2.pbm.license              \
             tests/goldenC3.pbm tests/goldenC3.pbm.license              \
             tests/goldenD1.ppm tests/goldenD1.ppm.license              \
             tests/goldenD2.ppm tests/goldenD2.ppm.license              \
             tests/goldenD3.ppm tests/goldenD3.ppm.license              \
             tests/goldenE001.pbm tests/goldenE001.pbm.license          \
             tests/goldenE002.pbm tests/goldenE002.pbm.license          \
             tests/goldenE003.pbm tests/goldenE003.pbm.license          \
             tests/goldenE004.pbm tests/goldenE004.pbm.license          \
             tests/goldenE005.pbm tests/goldenE005.pbm.license          \
             tests/goldenE006.pbm tests/goldenE006.pbm.license          \
             tests/goldenE201.pbm tests/goldenE201.pbm.license          \
             tests/goldenE202.pbm tests/goldenE202.pbm.license          \
             tests/goldenE301.pbm tests/goldenE301.pbm.license          \
             tests/goldenE302.pbm tests/goldenE302.pbm.license          \
             tests/goldenF11.pbm tests/goldenF11.pbm.license            \
             tests/goldenF21.pbm tests/goldenF21.pbm.license            \
             tests/goldenF3.pbm tests/goldenF3.pbm.license              \
             tests/goldenG1.pbm tests/goldenG1.pbm.license              \
             tests/goldenG2.pbm tests/goldenG2.pbm.license              \
             tests/goldenG3.pbm tests/goldenG3.pbm.license              \
             tests/imgsrc001.png tests/imgsrc001.png.license            \
             tests/imgsrc002.png tests/imgsrc002.png.license            \
             tests/imgsrc003.png tests/imgsrc003.png.license            \
             tests/imgsrc004.png tests/imgsrc004.png.license            \
             tests/imgsrc005.png tests/imgsrc005.png.license            \
             tests/imgsrcE001.png tests/imgsrcE001.png.license          \
             tests/imgsrcE002.png tests/imgsrcE002.png.license          \
             tests/imgsrcE003.png tests/imgsrcE003.png.license          \
             .dir-locals.el

CLEANFILES = tests/.dirstamp tests/resultsA1.pbm tests/resultsB1.ppm    \
	     tests/resultsB2.ppm tests/resultsB3.ppm                    \
	     tests/resultsC1.pbm tests/resultsC2.pbm                    \
	     tests/resultsC3.pbm tests/resultsD1.ppm                    \
	     tests/resultsD2.ppm tests/resultsD3.ppm                    \
	     tests/resultsE001.pbm tests/resultsE002.pbm                \
	     tests/resultsE003.pbm tests/resultsE004.pbm                \
	     tests/resultsE005.pbm tests/resultsE006.pbm                \
	     tests/resultsE201.pbm tests/resultsE202.pbm                \
	     tests/resultsE301.pbm tests/resultsE302.pbm                \
	     tests/resultsF11.pbm tests/resultsF21.pbm                  \
	     tests/resultsF3.pbm tests/resultsG1.pbm                    \
	     tests/resultsG2.pbm tests/resultsG3.pbm

%.1: %.1.xml
	$(AM_V_GEN)$(XSLTPROC) \
		--stringparam man.copyright.section.enable 0 \
		--stringparam man.authors.section.enabled 0 \
		$(MANPAGE_XSL) $<

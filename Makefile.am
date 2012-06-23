bin_PROGRAMS = unpaper
MANPAGE_XSL ?= http://docbook.sourceforge.net/release/xsl-ns/current/manpages/docbook.xsl

unpaper_SOURCES = \
	file.c file.h \
	imageprocess.c imageprocess.h \
	parse.c parse.h \
	tools.c tools.h \
	unpaper.c unpaper.h

dist_doc_DATA = NEWS COPYING README AUTHORS TODO
dist_man_MANS = unpaper.1

dist_html_DATA = doc/unpaper.html doc/index.html
imgdir = $(htmldir)/img
dist_img_DATA = doc/img/_after.png doc/img/_arrow.png			\
                doc/img/_before.png doc/img/blackfilter-detail.png	\
                doc/img/blackfilter.png doc/img/blurfilter-detail.png	\
                doc/img/blurfilter.png doc/img/border-align.png		\
                doc/img/border-scan.png doc/img/deskew-detail1.png	\
                doc/img/deskew-detail2.png doc/img/deskew.png		\
                doc/img/documentation.odg doc/img/input-pages.png	\
                doc/img/layout-templates.png doc/img/mask-center.png	\
                doc/img/mask-scan-detail1.png				\
                doc/img/mask-scan-detail2.png doc/img/mask-scan.png	\
                doc/img/multiple-input-files.png			\
                doc/img/multiple-output-files.png			\
                doc/img/noisefilter.png doc/img/nospam.js		\
                doc/img/output-pages.png doc/img/processing-order.png	\
                doc/img/sheetspages.png					\
                doc/img/standard-deviation.png doc/img/style.css	\
                doc/img/x-click-but04.gif

TESTS = tests/runtestA1.sh tests/runtestB1.sh tests/runtestB2.sh	\
        tests/runtestB3.sh tests/runtestC1.sh tests/runtestC2.sh	\
        tests/runtestC3.sh tests/runtestD1.sh tests/runtestD2.sh	\
        tests/runtestD3.sh tests/runtestE1.sh tests/runtestE2.sh	\
        tests/runtestE3.sh tests/runtestF1.sh tests/runtestF2.sh	\
        tests/runtestF3.sh

EXTRA_DIST = $(TESTS) tests/imgsrc001.png tests/imgsrc002.png	\
             tests/imgsrc003.png tests/imgsrc004.png		\
             tests/imgsrc005.png tests/imgsrcE001.png		\
             tests/imgsrcE002.png tests/imgsrcE003.png

check_DATA = tests/imgsrc001.pbm tests/imgsrc002.pbm	\
             tests/imgsrc003.ppm tests/imgsrc004.pgm	\
             tests/imgsrc005.pbm tests/imgsrcE001.pbm	\
             tests/imgsrcE002.pbm tests/imgsrcE003.pbm

CLEANFILES = $(check_DATA) tests/.dirstamp tests/resultsA1.pbm	\
	      tests/resultsB1.ppm   tests/resultsB2.ppm		\
	      tests/resultsB3.ppm   tests/resultsC1.pbm		\
	      tests/resultsC2.pbm   tests/resultsC3.pbm		\
	      tests/resultsD1.ppm   tests/resultsD2.ppm		\
	      tests/resultsD3.ppm   tests/resultsE001.pbm	\
	      tests/resultsE002.pbm tests/resultsE003.pbm	\
	      tests/resultsE004.pbm tests/resultsE005.pbm	\
	      tests/resultsE006.pbm tests/resultsE201.pbm	\
	      tests/resultsE202.pbm tests/resultsE301.pbm	\
	      tests/resultsE302.pbm

tests/.dirstamp:
	$(MKDIR_P) tests
	touch tests/.dirstamp

tests/imgsrc%.pbm tests/imgsrc%.ppm tests/imgsrc%.pgm: tests/imgsrc%.png tests/.dirstamp
	$(PNGTOPNM) $< > $@

tests/results%.png: tests/results%.pbm tests/.dirstamp
	$(PNMTOPNG) $< > $@

%.1: %.1.xml
	$(AM_V_GEN)$(XSLTPROC) \
		--stringparam man.copyright.section.enable 0 \
		--stringparam man.authors.section.enabled 0 \
		$(MANPAGE_XSL) $<

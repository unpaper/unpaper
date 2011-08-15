bin_PROGRAMS = unpaper

unpaper_SOURCES = unpaper.c

dist_doc_DATA = NEWS COPYING README AUTHORS TODO

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
        tests/runtestD1.sh tests/runtestD2.sh tests/runtestD3.sh	\
        tests/runtestE.sh

EXTRA_DIST = $(TESTS) tests/imgsrc001.png tests/imgsrc002.png	\
             tests/imgsrc003.png tests/imgsrc004.png		\
             tests/imgsrc005.png tests/imgsrcE001.png		\
             tests/imgsrcE002.png tests/imgsrcE003.png

check_DATA = tests/imgsrc001.pbm tests/imgsrc002.pbm	\
             tests/imgsrc003.ppm tests/imgsrc004.pgm	\
             tests/imgsrc005.pbm tests/imgsrcE001.pbm	\
             tests/imgsrcE002.pbm tests/imgsrcE003.pbm

CLEANFILES = $(check_DATA) tests/.dirstamp tests/resultsA1.pbm	\
             tests/resultsA1.txt tests/resultsB1.ppm		\
             tests/resultsB1.txt tests/resultsB2.ppm		\
             tests/resultsB2.txt tests/resultsB3.ppm		\
             tests/resultsB3.txt tests/resultsC1.pbm		\
             tests/resultsC1.txt tests/resultsC2.pbm		\
             tests/resultsC2.txt tests/resultsD1.ppm		\
             tests/resultsD1.txt tests/resultsD2.ppm		\
             tests/resultsD2.txt tests/resultsD3.ppm		\
             tests/resultsD3.txt tests/resultsE001.pbm		\
             tests/resultsE002.pbm tests/resultsE003.pbm	\
             tests/resultsE004.pbm tests/resultsE005.pbm	\
             tests/resultsE006.pbm tests/resultsE.txt

tests/.dirstamp:
	$(mkdir_p) tests
	touch tests/.dirstamp

tests/imgsrc%.pbm tests/imgsrc%.ppm tests/imgsrc%.pgm: tests/imgsrc%.png tests/.dirstamp
	$(PNGTOPNM) $< > $@

tests/results%.png: tests/results%.pbm tests/.dirstamp
	$(PNMTOPNG) $< > $@

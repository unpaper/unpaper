#!/bin/sh

echo "[B1] Combined Color/Gray, No Processing."

. tests/prologue.sh

rm -f tests/resultsB1.ppm
$UNPAPER -v --overwrite -n --input-pages 2 tests/imgsrc003.ppm tests/imgsrc004.pgm tests/resultsB1.ppm

[ -f tests/resultsB1.ppm ]

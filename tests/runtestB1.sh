#!/bin/sh

echo "[B1] Combined Color/Gray, No Processing."

set -e
set -x

rm -f tests/resultsB1.ppm
./unpaper -v -n --input-pages 2 tests/imgsrc003.ppm tests/imgsrc004.pgm tests/resultsB1.ppm

[ -f tests/resultsB1.ppm ]

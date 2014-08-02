#!/bin/sh

echo "[B1] Combined Color/Gray, No Processing."

set -e
set -x

rm -f tests/resultsB1.ppm
./unpaper -v -n --input-pages 2 ${srcdir:-.}/tests/imgsrc003.png ${srcdir:-.}/tests/imgsrc004.png tests/resultsB1.ppm

[ -f tests/resultsB1.ppm ]

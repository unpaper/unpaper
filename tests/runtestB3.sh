#!/bin/sh

echo "[B3] Combined Gray/Black+White, No Processing."

set -e
set -x

rm -f tests/resultsB3.ppm
./unpaper -v -n --input-pages 2 ${srcdir:-.}/tests/imgsrc004.png ${srcdir:-.}/tests/imgsrc005.png tests/resultsB3.ppm

[ -f tests/resultsB3.ppm ]

./compare-image ${srcdir:-.}/tests/goldenB3.ppm tests/resultsB3.ppm

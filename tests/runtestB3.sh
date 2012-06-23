#!/bin/sh

echo "[B3] Combined Gray/Black+White, No Processing."

set -e
set -x

rm -f tests/resultsB3.ppm
./unpaper -v -n --input-pages 2 tests/imgsrc004.pgm tests/imgsrc005.pbm tests/resultsB3.ppm

[ -f tests/resultsB3.ppm ]

#!/bin/sh

echo "[B2] Combined Color/Black+White, No Processing."

set -e
set -x

rm  -f tests/resultsB2.ppm
./unpaper -v -n --input-pages 2 tests/imgsrc003.ppm tests/imgsrc005.pbm tests/resultsB2.ppm

[ -f tests/resultsB2.ppm ]

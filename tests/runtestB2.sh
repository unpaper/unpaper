#!/bin/sh

echo "[B2] Combined Color/Black+White, No Processing."

set -e
set -x

rm  -f tests/resultsB2.ppm
./unpaper -v -n --input-pages 2 ${srcdir:-.}/tests/imgsrc003.png ${srcdir:-.}/tests/imgsrc005.png tests/resultsB2.ppm

[ -f tests/resultsB2.ppm ]

md5sum -c - <<EOF
996bb58805df0b55aeea80497c045e46  tests/resultsB2.ppm
EOF

#!/bin/sh

echo "[B3] Combined Gray/Black+White, No Processing."

set -e
set -x

rm -f tests/resultsB3.ppm
./unpaper -v -n --input-pages 2 ${srcdir:-.}/tests/imgsrc004.png ${srcdir:-.}/tests/imgsrc005.png tests/resultsB3.ppm

[ -f tests/resultsB3.ppm ]

md5sum -c - <<EOF
9a4fda67294b67060a2767fa54fcbc0a  tests/resultsB3.ppm
EOF

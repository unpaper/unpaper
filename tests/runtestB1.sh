#!/bin/sh

echo "[B1] Combined Color/Gray, No Processing."

set -e
set -x

rm -f tests/resultsB1.ppm
./unpaper -v -n --input-pages 2 ${srcdir:-.}/tests/imgsrc003.png ${srcdir:-.}/tests/imgsrc004.png tests/resultsB1.ppm

[ -f tests/resultsB1.ppm ]

md5sum -c - <<EOF
fcf41692e6e68b9056a9a6987e393b35  tests/resultsB1.ppm
EOF

#!/bin/sh

echo "[D2] Fit to sheet size."

set -e
set -x

rm -f tests/resultsD2.ppm
./unpaper -v -n --size 20cm,10cm ${srcdir:-.}/tests/imgsrc003.png tests/resultsD2.ppm

[ -f tests/resultsD2.ppm ]

md5sum -c - <<EOF
8a4c1072293a05f2c30427bb6cb55a5e  tests/resultsD2.ppm
EOF

#!/bin/sh

echo "[D3] Stretch to sheet size."

set -e
set -x

rm -f tests/resultsD3.ppm
./unpaper -v -n --stretch 20cm,10cm ${srcdir:-.}/tests/imgsrc003.png tests/resultsD3.ppm

[ -f tests/resultsD3.ppm ]

md5sum -c - <<EOF
a8cfdbfb6decddeba3532c45e2497354  tests/resultsD3.ppm
EOF

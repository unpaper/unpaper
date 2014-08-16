#!/bin/sh

echo "[D1] Crop to sheet size."

set -e
set -x

rm -f tests/resultsD1.ppm
./unpaper -v -n --sheet-size 20cm,10cm ${srcdir:-.}/tests/imgsrc003.png tests/resultsD1.ppm

[ -f tests/resultsD1.ppm ]

md5sum -c - <<EOF
b2f1ca4de27691ea367317ecf4aa945a  tests/resultsD1.ppm
EOF

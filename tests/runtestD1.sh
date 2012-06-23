#!/bin/sh

echo "[D1] Crop to sheet size."

set -e
set -x

rm -f tests/resultsD1.ppm
./unpaper -v -n --sheet-size 20cm,10cm tests/imgsrc003.ppm tests/resultsD1.ppm

[ -f tests/resultsD1.ppm ]

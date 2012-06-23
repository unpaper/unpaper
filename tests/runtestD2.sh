#!/bin/sh

echo "[D2] Fit to sheet size."

set -e
set -x

rm -f tests/resultsD2.ppm
./unpaper -v -n --size 20cm,10cm tests/imgsrc003.ppm tests/resultsD2.ppm

[ -f tests/resultsD2.ppm ]

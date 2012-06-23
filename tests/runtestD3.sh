#!/bin/sh

echo "[D3] Stretch to sheet size."

set -e
set -x

rm -f tests/resultsD3.ppm
./unpaper -v -n --stretch 20cm,10cm tests/imgsrc003.ppm tests/resultsD3.ppm

[ -f tests/resultsD3.ppm ]

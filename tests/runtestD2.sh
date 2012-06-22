#!/bin/sh

echo "[D2] Fit to sheet size."

. tests/prologue.sh

rm -f tests/resultsD2.ppm
$UNPAPER -v --overwrite -n --size 20cm,10cm tests/imgsrc003.ppm tests/resultsD2.ppm

[ -f tests/resultsD2.ppm ]

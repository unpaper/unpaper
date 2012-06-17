#!/bin/sh

echo "[D2] Fit to sheet size."

. tests/prologue.sh

$UNPAPER -v --overwrite -n --size 20cm,10cm tests/imgsrc003.ppm tests/resultsD2.ppm

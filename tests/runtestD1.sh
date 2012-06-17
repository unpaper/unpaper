#!/bin/sh

echo "[D1] Crop to sheet size."

. tests/prologue.sh

$UNPAPER -v --overwrite -n --sheet-size 20cm,10cm tests/imgsrc003.ppm tests/resultsD1.ppm

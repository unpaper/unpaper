#!/bin/sh

. tests/prologue.sh

$UNPAPER -v --overwrite -n --sheet-size 20cm,10cm tests/imgsrc003.ppm tests/resultsD1.ppm

echo "Done: [D1] Crop to sheet size."

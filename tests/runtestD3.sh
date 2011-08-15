#!/bin/sh

. tests/prologue.sh

$UNPAPER -v --overwrite -n --stretch 20cm,10cm tests/imgsrc003.ppm tests/resultsD3.ppm

echo "Done: [D3] Stretch to sheet size."

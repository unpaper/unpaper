#!/bin/sh

echo "[B1] Combined Color/Gray, No Processing."

. tests/prologue.sh

$UNPAPER -v --overwrite -n --input-pages 2 tests/imgsrc003.ppm tests/imgsrc004.pgm tests/resultsB1.ppm

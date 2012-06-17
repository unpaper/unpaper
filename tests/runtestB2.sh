#!/bin/sh

echo "[B2] Combined Color/Black+White, No Processing."

. tests/prologue.sh

$UNPAPER -v --overwrite -n --input-pages 2 tests/imgsrc003.ppm tests/imgsrc005.pbm tests/resultsB2.ppm

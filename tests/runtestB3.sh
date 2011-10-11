#!/bin/sh

. tests/prologue.sh

$UNPAPER -v --overwrite -n --input-pages 2 tests/imgsrc004.pgm tests/imgsrc005.pbm tests/resultsB3.ppm

echo "Done: [B3] Combined Gray/Black+White, No Processing."

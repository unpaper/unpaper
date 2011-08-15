#!/bin/sh

. tests/prologue.sh

$UNPAPER -v --overwrite -n --input-pages 2 --input-file-sequence tests/imgsrc003.ppm tests/imgsrc004.pgm --output-file-sequence tests/resultsB1.ppm

echo "Done: [B1] Combined Color/Gray, No Processing."

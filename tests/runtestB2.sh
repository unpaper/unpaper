#!/bin/sh

. tests/prologue.sh

$UNPAPER -v --overwrite -n --input-pages 2 --input-file-sequence tests/imgsrc003.ppm tests/imgsrc005.pbm --output-file-sequence tests/resultsB2.ppm

echo "Done: [B2] Combined Color/Black+White, No Processing."

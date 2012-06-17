#!/bin/sh

echo "[E] Splitting 2-page layout into seperate output pages."

. tests/prologue.sh

$UNPAPER -v --overwrite --layout double --output-pages 2 tests/imgsrcE%03d.pbm tests/resultsE%03d.pbm

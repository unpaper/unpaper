#!/bin/sh

. tests/prologue.sh

$UNPAPER -v --overwrite --layout double --output-pages 2 tests/imgsrcE%03d.pbm tests/resultsE%03d.pbm

echo "Done: [E] Splitting 2-page layout into seperate output pages."

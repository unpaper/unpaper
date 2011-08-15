#!/bin/sh

. tests/prologue.sh

$UNPAPER -v --overwrite -n --sheet-size a4 --sheet-background black tests/imgsrc002.pbm tests/resultsC1.pbm

echo "Done: [C1] Black sheet background color."

#!/bin/sh

echo "[C1] Black sheet background color."

. tests/prologue.sh

rm -f tests/resultsC1.pbm
$UNPAPER -v --overwrite -n --sheet-size a4 --sheet-background black tests/imgsrc002.pbm tests/resultsC1.pbm

[ -f tests/resultsC1.pbm ]

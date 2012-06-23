#!/bin/sh

echo "[C1] Black sheet background color."

set -e
set -x

rm -f tests/resultsC1.pbm
./unpaper -v -n --sheet-size a4 --sheet-background black tests/imgsrc002.pbm tests/resultsC1.pbm

[ -f tests/resultsC1.pbm ]

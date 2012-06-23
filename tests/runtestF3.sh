#!/bin/sh

echo "[F3] Merging 2-page layout into single output page (with explicit input and output)."

set -e
set -x

rm -f tests/resultsF3.pbm
./unpaper -v --layout double --input-pages 2 tests/imgsrcE001.pbm tests/imgsrcE002.pbm tests/resultsF3.pbm

[ -f tests/resultsF3.pbm ]

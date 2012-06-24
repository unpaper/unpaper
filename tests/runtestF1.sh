#!/bin/sh

echo "[F1] Merging 2-page layout into single output page (with input and output wildcard)."

set -e
set -x

rm -f tests/resultsF11.pbm

./unpaper --end-sheet 1 -v --layout double --input-pages 2 tests/imgsrcE%03d.pbm tests/resultsF1%d.pbm

[ -f tests/resultsF11.pbm ]

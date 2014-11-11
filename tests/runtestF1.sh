#!/bin/sh

echo "[F1] Merging 2-page layout into single output page (with input and output wildcard)."

set -e
set -x

rm -f tests/resultsF11.pbm

./unpaper --end-sheet 1 -v --layout double --input-pages 2 ${srcdir:-.}/tests/imgsrcE%03d.png tests/resultsF1%d.pbm

[ -f tests/resultsF11.pbm ]

./compare-image ${srcdir:-.}/tests/goldenF11.pbm tests/resultsF11.pbm

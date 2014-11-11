#!/bin/sh

echo "[F3] Merging 2-page layout into single output page (with explicit input and output)."

set -e
set -x

rm -f tests/resultsF3.pbm
./unpaper -v --layout double --input-pages 2 ${srcdir:-.}/tests/imgsrcE001.png ${srcdir:-.}/tests/imgsrcE002.png tests/resultsF3.pbm

[ -f tests/resultsF3.pbm ]

./compare-image ${srcdir:-.}/tests/goldenF3.pbm tests/resultsF3.pbm

#!/bin/sh

echo "[F2] Merging 2-page layout into single output page (with output wildcard only)."

set -e
set -x

rm -f tests/resultsF21.pbm
./unpaper -v --layout double --input-pages 2 ${srcdir:-.}/tests/imgsrcE001.png ${srcdir:-.}/tests/imgsrcE002.png tests/resultsF2%d.pbm

[ -f tests/resultsF21.pbm ]

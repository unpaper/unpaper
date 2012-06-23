#!/bin/sh

echo "[F2] Merging 2-page layout into single output page (with output wildcard only)."

set -e
set -x

rm -f tests/resultsF21.pbm
./unpaper -v --layout double --input-pages 2 tests/imgsrcE001.pbm tests/imgsrcE002.pbm tests/resultsF2%d.pbm

[ -f tests/resultsF21.pbm ]

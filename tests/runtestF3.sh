#!/bin/sh

echo "[F3] Merging 2-page layout into single output page (with explicit input and output)."

set -e
set -x

rm -f tests/resultsF3.pbm
./unpaper -v --layout double --input-pages 2 ${srcdir:-.}/tests/imgsrcE001.png ${srcdir:-.}/tests/imgsrcE002.png tests/resultsF3.pbm

[ -f tests/resultsF3.pbm ]

md5sum -c - <<EOF
08323895a7b1da2b3015e186299fa60d  tests/resultsF3.pbm
EOF

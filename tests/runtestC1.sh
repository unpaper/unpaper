#!/bin/sh

echo "[C1] Black sheet background color."

set -e
set -x

rm -f tests/resultsC1.pbm
./unpaper -v -n --sheet-size a4 --sheet-background black ${srcdir:-.}/tests/imgsrc002.png tests/resultsC1.pbm

[ -f tests/resultsC1.pbm ]

md5sum -c - <<EOF
a18da03b32ae0e13e89654a06660eed9  tests/resultsC1.pbm
EOF

#!/bin/sh

echo "[A1] Single-Page Template Layout, Black+White, Full Processing."

set -e
set -x

rm -f tests/resultsA1.pbm
./unpaper -v ${srcdir:-.}/tests/imgsrc001.png tests/resultsA1.pbm

[ -f tests/resultsA1.pbm ]

md5sum -c - <<EOF
d5a4a4d8a0b5727a12f233b433623db3  tests/resultsA1.pbm
EOF

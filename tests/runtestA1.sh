#!/bin/sh

echo "[A1] Single-Page Template Layout, Black+White, Full Processing."

set -e
set -x

rm -f tests/resultsA1.pbm
./unpaper -v ${srcdir:-.}/tests/imgsrc001.png tests/resultsA1.pbm

[ -f tests/resultsA1.pbm ]

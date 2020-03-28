#!/bin/sh

echo "[E1] Splitting 2-page layout into separate output pages (with input and output wildcard)."

set -e
set -x

for i in 1 2 3 4 5 6; do
    rm -f tests/resultsE1-0$i.pbm
done
./unpaper -v --layout double --output-pages 2 ${srcdir:-.}/tests/imgsrcE%03d.png tests/resultsE1-%02d.pbm

for i in 1 2 3 4 5 6; do
    [ -f tests/resultsE1-0$i.pbm ]

    ./compare-image ${srcdir:-.}/tests/goldenE1-0$i.pbm tests/resultsE1-0$i.pbm
done

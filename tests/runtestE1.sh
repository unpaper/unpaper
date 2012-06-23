#!/bin/sh

echo "[E1] Splitting 2-page layout into seperate output pages (with input and output wildcard)."

set -e
set -x

for i in 1 2 3 4 5 6; do
    rm -f tests/resultsE00$i.pbm
done
./unpaper -v --layout double --output-pages 2 tests/imgsrcE%03d.pbm tests/resultsE%03d.pbm

for i in 1 2 3 4 5 6; do
    [ -f tests/resultsE00$i.pbm ]
done

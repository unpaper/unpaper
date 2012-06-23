#!/bin/sh

echo "[E2] Splitting 2-page layout into seperate output pages (with output wildcard only)."

set -e
set -x

for i in 1 2; do
    rm -f tests/resultsE20$i.pbm
done
./unpaper -v --layout double --output-pages 2 tests/imgsrcE001.pbm tests/resultsE2%02d.pbm

for i in 1 2; do
    [ -f tests/resultsE20$i.pbm ]
done

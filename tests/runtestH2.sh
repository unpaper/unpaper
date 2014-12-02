#!/bin/sh

echo "[H2] multi index fails on uncomplete range"

set -e
set -x

for i in 1 2 3 4 5 6; do
    rm -f tests/resultsH100$i.pbm
done

./unpaper -v ${srcdir:-.}/tests/imgsrcH%03d.png tests/resultsH2%02d.pbm --replace-blank 1,2- --no-processing 1-4,6

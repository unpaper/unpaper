#!/bin/sh

echo "[H3] multi index fails on uncomplete index"

set -e
set -x

for i in 1 2 3 4 5 6; do
    rm -f tests/resultsH100$i.pbm
done

./unpaper -v ${srcdir:-.}/tests/source/imgsrcH%03d.png tests/resultsH3%02d.pbm --replace-blank 1, --no-processing 1-4,6

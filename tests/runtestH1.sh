#!/bin/sh

echo "[H1] multi-index testing"

set -e
set -x

for i in 1 2 3 4 5 6; do
    rm -f tests/resultsH10$i.pbm
done

./unpaper -v ${srcdir:-.}/tests/imgsrcH%03d.png tests/resultsH1%02d.pbm --replace-blank 2,3-4 --no-processing 1-4,6

for i in 1 2 3 4 5 6; do
    [ -f tests/resultsH10$i.pbm ]

    ./compare-image ${srcdir:-.}/tests/goldenH10$i.pbm tests/resultsH10$i.pbm
done

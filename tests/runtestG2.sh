#!/bin/sh

echo "[G2] No processing, with overwrite on an existing file."

set -e
set -x

rm -f tests/resultsG2.pbm
touch tests/resultsG2.pbm

./unpaper --overwrite -v --no-processing 1 tests/imgsrc001.pbm tests/resultsG2.pbm

[ -s tests/resultsG2.pbm ]

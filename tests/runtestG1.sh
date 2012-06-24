#!/bin/sh

echo "[G1] No processing, with overwrite on non-existing file."

set -e
set -x

rm -f tests/resultsG1.pbm

./unpaper --overwrite -v --no-processing 1 tests/imgsrc001.pbm tests/resultsG1.pbm

[ -f tests/resultsG1.pbm ]

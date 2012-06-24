#!/bin/sh

echo "[G3] No processing, without overwrite on an existing file."

set -e
set -x

rm -f tests/resultsG3.pbm
touch tests/resultsG3.pbm

./unpaper -v --no-processing 1 tests/imgsrc001.pbm tests/resultsG3.pbm

[ -s tests/resultsG3.pbm ]

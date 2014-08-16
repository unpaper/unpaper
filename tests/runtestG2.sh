#!/bin/sh

echo "[G2] No processing, with overwrite on an existing file."

set -e
set -x

rm -f tests/resultsG2.pbm
touch tests/resultsG2.pbm

./unpaper --overwrite -v --no-processing 1 ${srcdir:-.}/tests/imgsrc001.png tests/resultsG2.pbm

[ -s tests/resultsG2.pbm ]

md5sum -c - <<EOF
7efdcb1d3d531ae8a4117ae40c1616b4  tests/resultsG2.pbm
EOF

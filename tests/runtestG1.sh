#!/bin/sh

echo "[G1] No processing, with overwrite on non-existing file."

set -e
set -x

rm -f tests/resultsG1.pbm

./unpaper --overwrite -v --no-processing 1 ${srcdir:-.}/tests/imgsrc001.png tests/resultsG1.pbm

[ -f tests/resultsG1.pbm ]

md5sum -c - <<EOF
7efdcb1d3d531ae8a4117ae40c1616b4  tests/resultsG1.pbm
EOF

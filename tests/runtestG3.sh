#!/bin/sh

echo "[G3] No processing, without overwrite on an existing file."

set -e
set -x

rm -f tests/resultsG3.pbm
touch tests/resultsG3.pbm

./unpaper -v --no-processing 1 ${srcdir:-.}/tests/imgsrc001.png tests/resultsG3.pbm

[ -s tests/resultsG3.pbm ]

md5sum -c - <<EOF
d41d8cd98f00b204e9800998ecf8427e  tests/resultsG3.pbm
EOF

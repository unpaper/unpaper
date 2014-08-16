#!/bin/sh

echo "[C2] Explicit shifting."

set -e
set -x

rm -f tests/resultsC2.pbm
./unpaper -v -n --sheet-size a4 --pre-shift -5cm,9cm ${srcdir:-.}/tests/imgsrc002.png tests/resultsC2.pbm

[ -f tests/resultsC2.pbm ]

md5sum -c - <<EOF
9f25e744b241e8ea1a80f0600b1a2977  tests/resultsC2.pbm
EOF

#!/bin/sh

echo "[C3] Explicit -1 size shifting."

set -e
set -x

rm -f tests/resultsC3.pbm
./unpaper -v -n --sheet-size a4 --pre-shift -1cm ${srcdir:-.}/tests/imgsrc002.png tests/resultsC3.pbm

[ -f tests/resultsC3.pbm ]

md5sum -c - <<EOF
a0577952458d23184dcac97b9094576f  tests/resultsC3.pbm
EOF

#!/bin/sh

echo "[C2] Explicit shifting."

set -e
set -x

rm -f tests/resultsC2.pbm
./unpaper -v -n --sheet-size a4 --pre-shift -5cm,9cm tests/imgsrc002.pbm tests/resultsC2.pbm

[ -f tests/resultsC2.pbm ]

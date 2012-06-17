#!/bin/sh

echo "[C2] Explicit shifting."

. tests/prologue.sh

$UNPAPER -v --overwrite -n --sheet-size a4 --pre-shift -5cm,9cm tests/imgsrc002.pbm tests/resultsC2.pbm

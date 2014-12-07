#!/bin/sh

echo "[NoInputFile] input file doesn't exist"

set -e
set -x

./unpaper -v tests/NoInputFile.pbm tests/resultsNoInputFile.pbm

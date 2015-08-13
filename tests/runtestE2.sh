#!/bin/sh

echo "[E2] Splitting 2-page layout into seperate output pages (with output wildcard only)."

set -e
set -x

for i in 1 2; do
    rm -f tests/resultsE20$i.pbm
done
./unpaper -v --layout double --output-pages 2 ${srcdir:-.}/tests/imgsrcE001.png tests/resultsE2%02d.pbm

for i in 1 2; do
    [ -f tests/resultsE20$i.pbm ]
done

md5sum -c - <<EOF
a85fd0b279bc9ecaa9a5bed7e829c43f  tests/resultsE201.pbm
b8a47789baa71cd5b3be7a4353bfa17b  tests/resultsE202.pbm
EOF

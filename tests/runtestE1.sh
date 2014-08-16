#!/bin/sh

echo "[E1] Splitting 2-page layout into seperate output pages (with input and output wildcard)."

set -e
set -x

for i in 1 2 3 4 5 6; do
    rm -f tests/resultsE00$i.pbm
done
./unpaper -v --layout double --output-pages 2 ${srcdir:-.}/tests/imgsrcE%03d.png tests/resultsE%03d.pbm

for i in 1 2 3 4 5 6; do
    [ -f tests/resultsE00$i.pbm ]
done

md5sum -c - <<EOF
a85fd0b279bc9ecaa9a5bed7e829c43f  tests/resultsE001.pbm
b8a47789baa71cd5b3be7a4353bfa17b  tests/resultsE002.pbm
f0dceb4f90bd29f9fc88d60fe0573a50  tests/resultsE003.pbm
71bd0d497957688180700cb307b177d8  tests/resultsE004.pbm
64c068de638ba8eaa4a03d84ef5ed9b6  tests/resultsE005.pbm
bbacef519befd0b3356555430a3f645f  tests/resultsE006.pbm
EOF

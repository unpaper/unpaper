#!/bin/sh

. tests/prologue.sh

$UNPAPER --version > ${output_file}
$UNPAPER -v --overwrite --layout double --output-pages 2 tests/imgsrcE%03d.pbm tests/resultsE%03d.pbm >> ${output_file}

cat ${output_file}

echo "Done: [E] Splitting 2-page layout into seperate output pages."

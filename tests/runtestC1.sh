#!/bin/sh

. tests/prologue.sh

$UNPAPER --version > ${output_file}
$UNPAPER -v --overwrite -n --sheet-size a4 --sheet-background black tests/imgsrc002.pbm tests/resultsC1.pbm >> ${output_file}

cat ${output_file}

echo "Done: [C1] Black sheet background color."

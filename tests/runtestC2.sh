#!/bin/sh

. tests/prologue.sh

$UNPAPER --version > ${output_file}
$UNPAPER -v --overwrite -n --sheet-size a4 --pre-shift -5cm,9cm tests/imgsrc002.pbm tests/resultsC2.pbm >> ${output_file}

cat ${output_file}

echo "Done: [C2] Explicit shifting."

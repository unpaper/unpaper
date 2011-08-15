#!/bin/sh

. tests/prologue.sh

$UNPAPER --version > ${output_file}
$UNPAPER -v --overwrite -n --sheet-size a4 --pre-shift -1cm tests/imgsrc002.pbm tests/resultsC3.pbm >> ${output_file}

cat ${output_file}

echo "Done: [C3] Explicit -1 size shifting."

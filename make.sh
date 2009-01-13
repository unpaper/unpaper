# Simple makefile to build/install unpaper.
#
# Usage: make.sh [install]

INST_DIR=/usr/local/bin

if [[ "$1" == "install" ]]; then
  # must be root here
  if [[ "`whoami`" != "root" ]]; then
    echo "You should be root to copy 'unpaper' to '$INST_DIR'. Or try manually if you know what you are doing."
    exit
  fi
  if [[ ! -x unpaper ]]; then
    # not compiled yet
    ./make.sh
  fi
  echo "installing to $INST_DIR/unpaper"
  cp -v unpaper $INST_DIR
  exit
fi

# $CFLAGS may contain specific processor architecture information
# or optimization flags, e.g.:
# "-O3 -funroll-all-loops -fomit-frame-pointer -ftree-vectorize".

# -lm is required to link the math-libary which provides sin(), cos() etc.

echo "compiling"
gcc ${CFLAGS} -lm -o unpaper src/unpaper.c

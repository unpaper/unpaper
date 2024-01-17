// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "logging.h"

VerboseLevel verbose = VERBOSE_NONE;

void verboseLog(VerboseLevel level, const char *fmt, ...) {
  if (verbose < level)
    return;

  va_list vl;
  va_start(vl, fmt);
  vfprintf(stderr, fmt, vl);
  va_end(vl);
}

/**
 * Print an error and exit process
 */
void errOutput(const char *fmt, ...) {
  va_list vl;

  fprintf(stderr, "unpaper: error: ");

  va_start(vl, fmt);
  vfprintf(stderr, fmt, vl);
  va_end(vl);

  fprintf(stderr, "\nTry 'man unpaper' for more information.\n");

  exit(1);
}

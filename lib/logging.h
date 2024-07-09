// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

typedef enum {
  VERBOSE_QUIET = -1,
  VERBOSE_NONE = 0,
  VERBOSE_NORMAL = 1,
  VERBOSE_MORE = 2,
  VERBOSE_DEBUG = 3,
  VERBOSE_DEBUG_SAVE = 4
} VerboseLevel;

// TODO: stop exposing the global variable.
extern VerboseLevel verbose;

void verboseLog(VerboseLevel level, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
_Noreturn void errOutput(const char *fmt, ...)
    __attribute__((format(printf, 1, 2))) __attribute__((noreturn));

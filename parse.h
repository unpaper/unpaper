// Copyright © 2005-2007 Jens Gulden
// Copyright © 2011-2011 Diego Elio Pettenò
// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdbool.h>

/* --- tool functions for parameter parsing and verbose output ------------ */

int parseDirections(char *s);

const char *getDirections(int d);

int parseEdges(char *s);

void printEdges(int d);

void parseInts(char *s, int i[2]);

void parseSize(char *s, int i[2], int dpi);

int parseColor(char *s);

void parseFloats(char *s, float f[2]);

char *implode(char *buf, const char *s[], int cnt);

struct MultiIndex {
  int count;
  int *indexes;
};

void parseMultiIndex(const char *optarg, struct MultiIndex *multiIndex);

bool isInMultiIndex(int index, struct MultiIndex multiIndex);

/**
 * Tests whether 'index' is either part of multiIndex or excludeIndex.
 * (Throughout the application, excludeIndex generalizes several individual
 * multi-indices: if an entry is part of excludeIndex, it is treated as being
 * an entry of all other multiIndices, too.)
 */
static inline bool isExcluded(int index, struct MultiIndex multiIndex,
                              struct MultiIndex excludeIndex) {
  return (isInMultiIndex(index, excludeIndex) ||
          isInMultiIndex(index, multiIndex));
}

void printMultiIndex(struct MultiIndex multiIndex);

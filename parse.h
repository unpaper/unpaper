/*
 * This file is part of Unpaper.
 *
 * Copyright © 2005-2007 Jens Gulden
 * Copyright © 2011-2011 Diego Elio Pettenò
 *
 * Unpaper is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * Unpaper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/* --- tool functions for parameter parsing and verbose output ------------ */


int parseDirections(char* s);

void printDirections(int d);

int parseEdges(char* s);

void printEdges(int d);

void parseInts(char* s, int i[2]);

void parseSize(char* s, int i[2], int dpi);

int parseColor(char* s);

void parseFloats(char* s, float f[2]);

char* implode(char* buf, const char* s[], int cnt);

void parseMultiIndex(const char *optarg, int multiIndex[], int* multiIndexCount);

bool isInMultiIndex(int index, int multiIndex[MAX_MULTI_INDEX], int multiIndexCount);

bool isExcluded(int index, int multiIndex[MAX_MULTI_INDEX], int multiIndexCount, int excludeIndex[MAX_MULTI_INDEX], int excludeIndexCount);

void printMultiIndex(int multiIndex[MAX_MULTI_INDEX], int multiIndexCount);

bool masksOverlapAny(int m[EDGES_COUNT], int masks[MAX_MASKS][EDGES_COUNT], int masksCount);

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

const char *getDirections(int d);

int parseEdges(char* s);

void printEdges(int d);

void parseInts(char* s, int i[2]);

void parseSize(char* s, int i[2], int dpi);

int parseColor(char* s);

void parseFloats(char* s, float f[2]);

char* implode(char* buf, const char* s[], int cnt);

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
static inline bool isExcluded(int index, struct MultiIndex multiIndex, struct MultiIndex excludeIndex) {
    return ( isInMultiIndex(index, excludeIndex) || isInMultiIndex(index, multiIndex) );
}

void printMultiIndex(struct MultiIndex multiIndex);

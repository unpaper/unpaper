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

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "unpaper.h"

#include "parse.h"

/* --- constants ---------------------------------------------------------- */

// factors for conversion to inches
#define CM2IN 0.393700787
#define MM2IN CM2IN/10.0

#define MEASUREMENTS_COUNT 3
static const struct {
    char unit[4];
    float factor;
} MEASUREMENTS[MEASUREMENTS_COUNT] = {
    { "in", 1.0 },
    { "cm", CM2IN },
    { "mm", MM2IN }
};

// papersize alias names
#define PAPERSIZES_COUNT 10
static const struct {
    char name[24];
    float width;
    float height;
} PAPERSIZES[PAPERSIZES_COUNT] = {
    { "a5",
      14.8 * CM2IN,
      21.0 * CM2IN },
    { "a5-landscape",
      21.0 * CM2IN,
      14.8 * CM2IN },
    { "a4",
      21.0 * CM2IN,
      29.7 * CM2IN },
    { "a4-landscape",
      29.7 * CM2IN,
      21.0 * CM2IN },
    { "a3",
      29.7 * CM2IN,
      42.0 * CM2IN },
    { "a3-landscape",
      42.0 * CM2IN,
      29.7 * CM2IN },
    { "letter",
      8.5, 11.0 },
    { "letter-landscape",
      11.0, 8.5 },
    { "legal",
      8.5, 14.0 },
    { "legal-landscape",
      14.0, 8.5 }
};



/**
 * Parses a parameter string on occurrences of 'vertical', 'horizontal' or both.
 */
int parseDirections(char* s) {
    int dir = 0;
    if (strchr(s, 'h') != 0) { // (there is no 'h' in 'vertical'...)
        dir = 1<<HORIZONTAL;
    }
    if (strchr(s, 'v') != 0) { // (there is no 'v' in 'horizontal'...)
        dir |= 1<<VERTICAL;
    }
    if (dir == 0)
        errOutput("unknown direction name '%s', expected 'h[orizontal]' or 'v[ertical]'.", s);

    return dir;
}


/**
 * Prints whether directions are vertical, horizontal, or both.
 */
const char *getDirections(int d) {
    switch(d) {
    case (1<<VERTICAL)|(1<<HORIZONTAL):
        return "[vertical,horizontal]";
    case (1<<VERTICAL):
        return "[vertical]";
    case (1<<HORIZONTAL):
        return "[horizontal]";
    }

    return "[none]";
}


/**
 * Parses a parameter string on occurrences of 'left', 'top', 'right', 'bottom' or combinations.
 */
int parseEdges(char* s) {
    int dir = 0;
    if (strstr(s, "left") != 0) {
        dir = 1<<LEFT;
    }
    if (strstr(s, "top") != 0) {
        dir |= 1<<TOP;
    }
    if (strstr(s, "right") != 0) {
        dir |= 1<<RIGHT;
    }
    if (strstr(s, "bottom") != 0) {
        dir |= 1<<BOTTOM;
    }
    if (dir == 0)
        errOutput("unknown edge name '%s', expected 'left', 'top', 'right' or 'bottom'.", s);

    return dir;
}


/**
 * Prints whether edges are left, top, right, bottom or combinations.
 */
void printEdges(int d) {
    bool comma = false;

    printf("[");
    if ((d & 1<<LEFT) != 0) {
        printf("left");
        comma = true;
    }
    if ((d & 1<<TOP) != 0) {
        if (comma == true) {
            printf(",");
        }
        printf("top");
        comma = true;
    }
    if ((d & 1<<RIGHT) != 0) {
        if (comma == true) {
            printf(",");
        }
        printf("right");
        comma = true;
    }
    if ((d & 1<<BOTTOM) != 0) {
        if (comma == true) {
            printf(",");
        }
        printf("bottom");
    }
    printf("]\n");
}


/**
 * Parses either a single integer string, of a pair of two integers seperated
 * by a comma.
 */
void parseInts(char* s, int i[2]) {
    i[0] = -1;
    i[1] = -1;
    sscanf(s, "%d,%d", &i[0], &i[1]);
    if (i[1]==-1) {
        i[1] = i[0]; // if second value is unset, copy first one into
    }
}


static int parseSizeSingle(const char *s, int dpi) {
    char *valueEnd;
    float value;

    value = strtof(s, &valueEnd);

    if ( fabs(value) == HUGE_VAL || s == valueEnd )
        errOutput("invalid size %s", s);

    for (int j = 0; j < MEASUREMENTS_COUNT; j++)
        if ( strcmp(valueEnd, MEASUREMENTS[j].unit) == 0 )
            return (int)(value * MEASUREMENTS[j].factor * dpi);

    /* if no unit is found, then we have a direct pixel value, do not
       multiply for dpi. */
    return (int)value;
}

/**
 * Parses a pair of size-values and returns it in pixels.
 * Values may be suffixed by MEASUREMENTS such as 'cm', 'in', in that case
 * conversion to pixels is perfomed based on the dpi-value.
 */
void parseSize(char* s, int i[2], int dpi) {
    char str[255];
    char* comma;
    int pos;

    // is s a papersize name?
    for (int j = 0; j < PAPERSIZES_COUNT; j++) {
        if (strcmp(s, PAPERSIZES[j].name)==0) {
            i[0] = PAPERSIZES[j].width * dpi;
            i[1] = PAPERSIZES[j].height * dpi;
            return;
        }
    }

    // find comma in size string, if there
    comma = strchr(s, ',');

    if (comma == NULL) {
        i[0] = i[1] = parseSizeSingle(s, dpi);
        return;
    }

    pos = comma - s;
    strncpy(str, s, pos);
    str[pos] = 0; // (according to spec of strncpy, no terminating 0 is written)

    i[0] = parseSizeSingle(str, dpi);

    strcpy(str, &s[pos+1]); // copy rest after ','
    i[1] = parseSizeSingle(str, dpi);
}


/**
 * Parses a color. Currently only "black" and "white".
 */
int parseColor(char* s) {
    if ( strcmp(s, "black") == 0 )
        return BLACK24;
    if ( strcmp(s, "white") == 0 )
      return WHITE24;

    errOutput("cannot parse color '%s'.", s);
}


/**
 * Parses either a single float string, of a pair of two floats seperated
 * by a comma.
 */
void parseFloats(char* s, float f[2]) {
    f[0] = -1.0;
    f[1] = -1.0;
    sscanf(s, "%f,%f", &f[0], &f[1]);
    if (f[1]==-1.0) {
        f[1] = f[0]; // if second value is unset, copy first one into
    }
}

/**
 * Combines an array of strings to a comma-seperated string.
 */
char* implode(char* buf, const char* s[], int cnt) {
    if (cnt > 0) {
        if (s[0] != NULL) {
            strcpy(buf, s[0]);
        } else {
            strcpy(buf, BLANK_TEXT);
        }
        for (int i = 1; i < cnt; i++) {
            if (s[i] != NULL) {
                sprintf(buf + strlen(buf), ", %s", s[i]);
            } else {
                sprintf(buf + strlen(buf), ", %s", BLANK_TEXT);
            }
        }
    } else {
        buf[0] = 0; //strcpy(buf, "");
    }
    return buf;
}

/**
 * Parses a string consisting of comma-concatenated integers. The
 * string may also be of a different format, in which case
 * *multiIndexCount is set to -1.
 *
 * @see isInMultiIndex(..)
 */
void parseMultiIndex(const char *optarg, struct MultiIndex *multiIndex) {
    char *s1, *s2 = NULL;
    char c;
    int index;
    int allocated = 0;

    multiIndex->count = -1;
    multiIndex->indexes = NULL;

    if ( optarg == NULL ) {
        return;
    }

    multiIndex->count = 0;
    allocated = 32;
    multiIndex->indexes = calloc(allocated, sizeof(multiIndex->indexes[0]));
    s1 = strdup(optarg);

    do {
        index = -1;
        sscanf(s1, "%d%c%ms", &index, &c, &s2);
        if (index != -1) {
            if (multiIndex->count >= allocated) {
                allocated += 32;
                multiIndex->indexes = realloc(multiIndex->indexes, allocated * sizeof(multiIndex->indexes[0]));
            }

            multiIndex->indexes[(multiIndex->count)++] = index;
            if (c=='-') { // range is specified: get range end
                strcpy(s1, s2); // s2 -> s1
                sscanf(s1, "%d,%s", &index, s2);
                size_t count = index - multiIndex->indexes[(multiIndex->count)-1];

                allocated += count;
                multiIndex->indexes = realloc(multiIndex->indexes, allocated);

                for (int j = multiIndex->indexes[(multiIndex->count)-1]+1; j <= index; j++) {
                    multiIndex->indexes[(multiIndex->count)++] = j;
                }
            }
        } else {
            // string is not correctly parseable: break without inreasing *i (string may be e.g. input-filename)
            multiIndex->count = -1; // disable all
            free(s1);
            free(s2);
            return;
        }
        if ( s2 ) {
            strcpy(s1, s2); // s2 -> s1
            free(s2);
        }
    } while ((multiIndex->count < MAX_MULTI_INDEX) && (strlen(s1) > 0));

    free(s1);
}


/**
 * Tests whether an integer is included in the array of integers given as multiIndex.
 * If multiIndexCount is -1, each possible integer is considered to be in the
 * multiIndex list.
 *
 * @see parseMultiIndex(..)
 */
bool isInMultiIndex(int index, struct MultiIndex multiIndex) {
    if (multiIndex.count == -1) {
        return true; // all
    } else {
        for (int i = 0; i < multiIndex.count; i++) {
            if (index == multiIndex.indexes[i]) {
                return true; // found in list
            }
        }
        return false; // not found in list
    }
}

/**
 * Outputs all entries in an array of integer to the console.
 */
void printMultiIndex(struct MultiIndex multiIndex) {
    if (multiIndex.count == -1) {
        printf("all");
    } else if (multiIndex.count == 0) {
        printf("none");
    } else {
        for (int i = 0; i < multiIndex.count; i++) {
            printf("%d", multiIndex.indexes[i]);
            if (i < multiIndex.count-1) {
                printf(",");
            }
        }
    }
    printf("\n");
}

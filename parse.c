/* ---------------------------------------------------------------------------
unpaper - written by Jens Gulden 2005-2007                                  */

/* ------------------------------------------------------------------------ */

/* --- tool functions for parameter parsing and verbose output ------------ */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "unpaper.h"

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
int parseDirections(char* s, int* exitCode) {
    int dir = 0;
    if (strchr(s, 'h') != 0) { // (there is no 'h' in 'vertical'...)
        dir = 1<<HORIZONTAL;
    }
    if (strchr(s, 'v') != 0) { // (there is no 'v' in 'horizontal'...)
        dir |= 1<<VERTICAL;
    }
    if (dir == 0) {
        printf("*** error: Unknown direction name '%s', expected 'h[orizontal]' or 'v[ertical]'.\n", s);
        *exitCode = 1;
    }
    return dir;
}


/**
 * Prints whether directions are vertical, horizontal, or both.
 */            
void printDirections(int d) {
    bool comma = false;
    
    printf("[");
    if ((d & 1<<VERTICAL) != 0) {
        printf("vertical");
        comma = true;
    }
    if ((d & 1<<HORIZONTAL) != 0) {
        if (comma == true) {
            printf(",");
        }
        printf("horizontal");
    }
    printf("]\n");
}


/**
 * Parses a parameter string on occurrences of 'left', 'top', 'right', 'bottom' or combinations.
 */            
int parseEdges(char* s, int* exitCode) {
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
    if (dir == 0) {
        printf("*** error: Unknown edge name '%s', expected 'left', 'top', 'right' or 'bottom'.\n", s);
        *exitCode = 1;
    }
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


static int parseSizeSingle(const char *s, int dpi, int *exitCode) {
    int j;
    char *valueEnd;
    float value;

    value = strtof(s, &valueEnd);

    if ( fabs(value) == HUGE_VAL || s == valueEnd ) {
        *exitCode = 1;
        return 0;
    }

    for (j = 0; j < MEASUREMENTS_COUNT; j++)
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
void parseSize(char* s, int i[2], int dpi, int* exitCode) {
    char str[255];
    int j;
    char* comma;
    int pos;

    // is s a papersize name?
    for (j = 0; j < PAPERSIZES_COUNT; j++) {
        if (strcmp(s, PAPERSIZES[j].name)==0) {
            i[0] = PAPERSIZES[j].width * dpi;
            i[1] = PAPERSIZES[j].height * dpi;
            return;
        }
    }

    // find comma in size string, if there
    comma = strchr(s, ',');    

    if (comma == NULL) {
        i[0] = i[1] = parseSizeSingle(s, dpi, exitCode);
        return;
    }

    pos = comma - s;
    strncpy(str, s, pos);
    str[pos] = 0; // (according to spec of strncpy, no terminating 0 is written)

    i[0] = parseSizeSingle(str, dpi, exitCode);
    if ( *exitCode == 1 ) return;

    strcpy(str, &s[pos+1]); // copy rest after ','
    i[1] = parseSizeSingle(str, dpi, exitCode);
    if ( *exitCode == 1 ) return;
}


/**
 * Parses a color. Currently only "black" and "white".
 */            
int parseColor(char* s, int* exitCode) {
    if ( strcmp(s, "black") == 0 )
        return BLACK;
    if ( strcmp(s, "white") == 0 )
        return WHITE;

    printf("*** error: cannot parse color '%s'.\n", s);
    *exitCode = 1;
    return WHITE;
}


/**
 * Outputs a pair of two integers seperated by a comma.
 */            
void printInts(int i[2]) {
    printf("[%d,%d]\n", i[0], i[1]);
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
 * Outputs a pair of two floats seperated by a comma.
 */            
void printFloats(float f[2]) {
    printf("[%f,%f]\n", f[0], f[1]);
}


/**
 * Combines an array of strings to a comma-seperated string.
 */
char* implode(char* buf, const char* s[], int cnt) {
    int i;
    if (cnt > 0) {
        if (s[0] != NULL) {
            strcpy(buf, s[0]);
        } else {
            strcpy(buf, BLANK_TEXT);
        }
        for (i = 1; i < cnt; i++) {        
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
 * Parses a string at argv[*i] argument consisting of comma-concatenated 
 * integers. The string may also be of a different format, in which case
 * *i remains unchanged and *multiIndexCount is set to -1.
 *
 * @see isInMultiIndex(..)
 */
void parseMultiIndex(int* i, char* argv[], int multiIndex[], int* multiIndexCount) {
    char s1[MAX_MULTI_INDEX * 5]; // buffer
    char s2[MAX_MULTI_INDEX * 5]; // buffer
    char c;
    int index;
    int j;
    
    (*i)++;
    *multiIndexCount = 0;
    if (argv[*i][0] != '-') { // not another option directly following
        strcpy(s1, argv[*i]); // argv[*i] -> s1
        do {
            index = -1;
            s2[0] = (char)0; // = ""
            sscanf(s1, "%d%c%s", &index, &c, s2);
            if (index != -1) {
                multiIndex[(*multiIndexCount)++] = index;
                if (c=='-') { // range is specified: get range end
                    strcpy(s1, s2); // s2 -> s1
                    sscanf(s1, "%d,%s", &index, s2);
                    for (j = multiIndex[(*multiIndexCount)-1]+1; j <= index; j++) {
                        multiIndex[(*multiIndexCount)++] = j;
                    }
                }
            } else {
                // string is not correctly parseable: break without inreasing *i (string may be e.g. input-filename)
                *multiIndexCount = -1; // disable all
                (*i)--;
                return; // exit here
            }
            strcpy(s1, s2); // s2 -> s1
        } while ((*multiIndexCount < MAX_MULTI_INDEX) && (strlen(s1) > 0));
    } else { // no explicit list of sheet-numbers given
        *multiIndexCount = -1; // disable all
        (*i)--;
        return;
    }
}


/**
 * Tests whether an integer is included in the array of integers given as multiIndex.
 * If multiIndexCount is -1, each possible integer is considered to be in the
 * multiIndex list.
 *
 * @see parseMultiIndex(..)
 */
bool isInMultiIndex(int index, int multiIndex[MAX_MULTI_INDEX], int multiIndexCount) {
    int i;
    
    if (multiIndexCount == -1) {
        return true; // all
    } else {
        for (i = 0; i < multiIndexCount; i++) {
            if (index == multiIndex[i]) {
                return true; // found in list
            }
        }
        return false; // not found in list
    }
}


/**
 * Tests whether 'index' is either part of multiIndex or excludeIndex.
 * (Throughout the application, excludeIndex generalizes several individual 
 * multi-indices: if an entry is part of excludeIndex, it is treated as being
 * an entry of all other multiIndices, too.)
 */
bool isExcluded(int index, int multiIndex[MAX_MULTI_INDEX], int multiIndexCount, int excludeIndex[MAX_MULTI_INDEX], int excludeIndexCount) {
    return ( (isInMultiIndex(index, excludeIndex, excludeIndexCount) == true) || (isInMultiIndex(index, multiIndex, multiIndexCount) == true) );
}


/**
 * Outputs all entries in an array of integer to the console.
 */
void printMultiIndex(int multiIndex[MAX_MULTI_INDEX], int multiIndexCount) {
    int i;
    
    if (multiIndexCount == -1) {
        printf("all");
    } else if (multiIndexCount == 0) {
        printf("none");
    } else {
        for (i = 0; i < multiIndexCount; i++) {
            printf("%d", multiIndex[i]);
            if (i < multiIndexCount-1) {
                printf(",");
            }
        }
    }
    printf("\n");
}


/**
 * Tests if a point is covered by a mask.
 */
static bool inMask(int x, int y, int mask[EDGES_COUNT]) {
    if ( (x >= mask[LEFT]) && (x <= mask[RIGHT]) && (y >= mask[TOP]) && (y <= mask[BOTTOM]) ) {
        return true;
    } else {
        return false;
    }
}


/**
 * Tests if masks a and b overlap.
 */
static bool masksOverlap(int a[EDGES_COUNT], int b[EDGES_COUNT]) {
    return ( inMask(a[LEFT], a[TOP], b) || inMask(a[RIGHT], a[BOTTOM], b) );
}


/**
 * Tests if at least one mask in masks overlaps with m.
 */
bool masksOverlapAny(int m[EDGES_COUNT], int masks[MAX_MASKS][EDGES_COUNT], int masksCount) {
    int i;
    
    for ( i = 0; i < masksCount; i++ ) {
        if ( masksOverlap(m, masks[i]) ) {
            return true;
        }
    }
    return false;
}

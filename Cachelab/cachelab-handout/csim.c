#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define BUFSIZE 80
/*
 * Maybe there is a bug in test-csim, try type ./csim -s 4 -E 2 -b 4  -t traces/yi.trace
 * before you run the autograding program(like ./driver.py).
 */


// Data structure for each record of file.trace
typedef struct DataRecord {
    unsigned long address;
    int bytes;
    char type;
} DataRecord;

// Data structure for one unit of cache.
typedef struct CacheUnit {
    unsigned long tag;
    unsigned long set; // set index
    int valid;
    time_t usetime; // recently used time
} CacheUnit;


typedef CacheUnit *CacheSet; // CacheSet is an array (first address) that contains Enum Cacheunits
typedef CacheSet *Cache;     // Cache is an array (first address) that contains 2^s_bit Cachesets

// Initate the global parameters.
int s_bit = -1; // number of set index bits
int E_num = -1; // number of lines per set
int b_bit = -1; // number of block offset bits
int hitcount = 0, misscount = 0, evictcount = 0;
int vbose = 0;  // verbose flag


// Convert an address(string) to an unsigned long number.
unsigned long ConvertAddress(char *str) {
    unsigned long result = 0;
    int n = strlen(str);
    for (int i = 0; i < n; i++) {
        char ch = str[n - 1 - i];
        if (isdigit(ch)) {
            result += (((unsigned long)1 << (unsigned long)(4*i)) * (unsigned long)(ch - '0')); // (1 << (4*i)) = 16^i
        } else if (isalpha(ch)) {
            ch = tolower(ch);
            result += (((unsigned long)1 << (unsigned long)(4*i)) * (unsigned long)(ch - 'a' + 10));
        }
    }
    return result;
}

// Print the usage message.
void PrintUsage(char *filename) {
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", filename);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n\n");
    printf("Examples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", filename);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", filename);
}

// Error handling.
void MyError(char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(0);
}

// Error handling for miss command.
void PrintMissCommand(char *filename) {
    printf("%s: Missing required command line argument\n", filename);
    PrintUsage(filename);
    exit(0);
}

// Parse a data record and check the situation of cache.
void CheckDataRecord(DataRecord data, Cache cache_array) {
    unsigned long address = data.address >> b_bit; // No need for the b_bit in this lab.
    unsigned long setmask = 0; // use setmask to extract the set bits of data.
    for (int i = 0; i < s_bit; i++) {
        setmask += 0x1 << i;
    }
    unsigned long data_set = address & setmask; // get the data's set index.
    unsigned long data_tag = address - data_set; // get the data's tag number.
    
    int emptyindex = -1; // the index for an empty cacheunit.
    int leastindex = -1; // the index for the least recently used cacheunit.
    time_t leasttime = 0x3fffffff; // INF
    for (int j = 0; j < E_num; j++) { // check all the lines of one set.
        if (cache_array[data_set][j].valid == 1 && cache_array[data_set][j].tag == data_tag) { // if find the tag and is valid
            hitcount++;
            if (vbose) {
                printf(" hit");
            }
            cache_array[data_set][j].usetime = clock(); // record the time.
            goto Done; // if find, no need to check other units.
        } else if (cache_array[data_set][j].valid == 0) { // record the index of an empty unit.
            emptyindex = j;
        } else {
            if (cache_array[data_set][j].usetime < leasttime) { // record the index for the least recently used cacheunit.
                leasttime = cache_array[data_set][j].usetime;
                leastindex = j;
            }
        }
    }
    if (emptyindex >= 0) { // if not find, first check if there is an empty unit.
        cache_array[data_set][emptyindex].valid = 1;
        cache_array[data_set][emptyindex].tag = data_tag;
        cache_array[data_set][emptyindex].usetime = clock();
        misscount++;
        if (vbose) {
            printf(" miss");
        }
    } else { // if not find and no empty units, evict the least recently used unit.
        cache_array[data_set][leastindex].tag = data_tag;
        cache_array[data_set][leastindex].usetime = clock();
        misscount++;
        evictcount++;
        if (vbose) {
            printf(" miss eviction");
        }        
    }

Done:
    if (data.type == 'M') { // deal with the M type, just add one hit count.
        hitcount++;
        if (vbose) {
            printf(" hit");
        }
    }
    if (vbose) {
        printf("  Set: %lu\n", data_set);
    }
}

// Parse the command line, return the trace file's name (address).
char *ParseTheCommand(int argc, char **argv) {
    char *filename;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') { // if the first char is '-', then is valid for now
            char ch = argv[i][1]; // check the next char
            if (ch == 'h') { // if encounter "-h", just print the help message
                PrintUsage(argv[0]);
                exit(0);
            } else if (ch == 's' || ch == 'E' || ch == 'b') { // record cache parameters
                int n = strlen(argv[i + 1]);
                for (int j = 0; j < n; j++) {
                    if (!isdigit(argv[i + 1][j])) {
                        PrintMissCommand(argv[0]);
                    }
                }
                int num_tmp = atoi(argv[i + 1]);
                switch (ch) {
                    case 's':
                        s_bit = num_tmp;
                        break;
                    case 'E':
                        E_num = num_tmp;
                        break;
                    case 'b':
                        b_bit = num_tmp;
                        break;
                }
                i++;
            } else if (ch == 't') { // record the trace file's name
                filename = argv[i + 1];
                i++;
            } else if (ch == 'v') { // set the verbose flag
                vbose = 1;
            } else {
                printf("%s: invalid option -- '%c'\n", argv[0], ch);
                PrintUsage(argv[0]);
                exit(0);
            }
        } else { // if first char isn't '-', then is invalid
            PrintMissCommand(argv[0]);
        }
    }
    if (s_bit < 0 || E_num < 0 || b_bit < 0) { // if miss any cache parameters
        PrintMissCommand(argv[0]);
    }
    return filename;
}

// Create and initate a cache according to the cache parameters.
Cache InitCache() {
    int s_num = 1 << s_bit; // s_num = 2^(s_bit)
    Cache cache_array = (Cache) malloc(s_num * sizeof(CacheSet));
    if (cache_array == NULL) {
        MyError("No enough memory!");
    }
    for (int i = 0; i < s_num; i++) {
        cache_array[i] = (CacheSet) calloc(E_num, sizeof(CacheUnit));
        if (cache_array[i] == NULL) {
            MyError("No enough memory!");
        }
    }
    return cache_array;
}

// Delete the cache and free the memory.
void DeleteCache(Cache cache_array) {
    int s_num = 1 << s_bit; // s_num = 2^(s_bit)
    if (cache_array != NULL) {
        for (int i = 0; i < s_num; i++) {
            if (cache_array[i] != NULL) {
                free(cache_array[i]);
            }
        }
        free(cache_array);
    }
}

// Parse each line of the trace file.
void ParseTheLine(char *filename, Cache cache_array) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("%s: No such file or directory\n", filename);
        exit(0);
    }
    char linestr[BUFSIZE];
    while (1) { // avoid fgets() getting the last line twice when end with an empty line.
        fgets(linestr, BUFSIZE, fp);
        if (feof(fp)) {
            break;
        }
        if (linestr[0] == ' ') { // if first char isn't space, ignore it.
            DataRecord tmp;
            tmp.type = linestr[1];
            int i = 3; // the index of first address char
            while (linestr[i] != ',') {
                i++;
            }
            tmp.bytes = (int) (linestr[i + 1] - '0'); // the byte is record after ','
            linestr[i] = '\0'; // change the end of the address to make it become a string
            tmp.address = ConvertAddress(linestr + 3); // convert string to unsigned long
            if (vbose) {
                printf("%c %s,%d", tmp.type, linestr + 3, tmp.bytes);
            }
            CheckDataRecord(tmp, cache_array);
        }

    }
    fclose(fp);
    return;

}

int main(int argc, char **argv)
{
    char* filename = ParseTheCommand(argc, argv);
    Cache cache_array = InitCache();
    ParseTheLine(filename, cache_array);
    printSummary(hitcount, misscount, evictcount);
    DeleteCache(cache_array);
    return 0;
}

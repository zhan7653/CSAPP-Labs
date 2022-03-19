#include <stdio.h>

typedef long long int word_t;

word_t src[8] = {1, 2, 3, 4, 5, 6, -1, -2};
word_t dst[8] = {11, 22, 33, 44, 55, 66, 77, 88};

/* $begin ncopy */
/*
 * ncopy - copy src to dst, returning number of positive ints
 * contained in src array.
 */
word_t ncopy(word_t *src, word_t *dst, word_t len)
{
    word_t count = 0;
    word_t val1, val2;
    word_t limit = len - 1;
    word_t i;

    for (i = 0; i < limit; i++) {
        val1 = src[i];
        dst[i] = val1;
        if (val1 > 0) {
            count++;
        }
        val2 = src[i + 1];
        dst[i + 1] = val2;
        if (val2 > 0) {
            count++;
        }        
    }
    for (; i < len; i++) {
        val1 = src[i];
        dst[i] = val1;
        if (val1 > 0) {
            count++;
        }        
    }
    return count;
}
/* $end ncopy */

int main() {
    ncopy(src, dst, 8);
    return 0;
}


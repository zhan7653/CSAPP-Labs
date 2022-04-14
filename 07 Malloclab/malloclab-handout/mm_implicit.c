/*
 * Implicit free list on the textbook.
 *
 * Results for mm malloc:
 * trace  valid  util     ops      secs  Kops
 *  0       yes   99%    5694  0.007243   786
 *  1       yes   99%    5848  0.006488   901
 *  2       yes   99%    6648  0.010598   627
 *  3       yes  100%    5380  0.007975   675
 *  4       yes   66%   14400  0.000078183439
 *  5       yes   92%    4800  0.006981   688
 *  6       yes   92%    4800  0.006291   763
 *  7       yes   55%   12000  0.156557    77
 *  8       yes   51%   24000  0.265339    90
 *  9       yes   27%   14401  0.064317   224
 * 10       yes   34%   14401  0.002096  6870
 * Total          74%  112372  0.533964   210
 * 
 * Perf index = 44 (util) + 14 (thru) = 58/100
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "From7heAbyss",
    /* First member's full name */
    "Alan Zhang",
    /* First member's email address */
    "zhan7653@foxmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* Basic constants and macros */
#define ALIGNMENT 8 // single word (4) or double word (8) alignment
#define WSIZE 4     // word and header/footer size (bytes)
#define DSIZE 8     // double word size (bytes)
#define CHUNKSIZE (1<<12)   // extend heap by this default amount (bytes)

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)  
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Pack a size and allocate bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)  (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDPR(bp) ((char *)(bp) - WSIZE)
#define FTPR(bp) ((char *)(bp) + GET_SIZE(HDPR(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

/* Global variables */
static char *heap_listp = 0;    // pointer to first block

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *first_fit(size_t size);
static void place(void *bp, size_t size);



/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) {
        printf("mm_init fault!\n");
        return -1;
    }
    PUT(heap_listp, 0);                             // alignment padding
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));    // prologue header
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    // prologue footer
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));    // epilogue header
    heap_listp += (2*WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        return -1;
    }

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;

    /* Ignore spurious requests */
    if (size == 0) {
        return NULL;
    }
    
    /* Adjust block size to include overhead and alignment reqs */
    if (size <= DSIZE) {
        asize = 2*DSIZE;
    } else {
        asize = ALIGN(size + SIZE_T_SIZE);
    }

    /* Search the free list for a fit */
    if ((bp = first_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL) {
        return NULL;
    }
    place(bp, asize);
    return bp;
    
}

/*
 * mm_free - Freeing a block and coalescing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDPR(bp));

    PUT(HDPR(bp), PACK(size, 0));
    PUT(FTPR(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *bp, size_t size)
{
    void *oldptr = bp;
    void *newptr;
    size_t copySize = GET_SIZE(HDPR(bp));

    /* If bp is NULL, the call is equivalent to mm_malloc(size) */
    if (oldptr == NULL) {
        return mm_malloc(size);
    }

    /* If size is equal to zero, the call is equivalent to mm_free(bp) */
    if (size == 0) {
        mm_free(oldptr);
        return NULL;
    }

    /* Otherwise, need to realloc */
    if ((newptr = mm_malloc(size)) == NULL) {
        return NULL;
    }
    if (size < copySize) {
        copySize = size;
    }
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}




/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2)? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1) {
        printf("extend_heap fault!\n");
        return NULL;
    }

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDPR(bp), PACK(size, 0));
    PUT(FTPR(bp), PACK(size, 0));
    PUT(HDPR(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}


/*
 * coalesce - Boundry tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp) {
    size_t pre_alloc = GET_ALLOC(HDPR(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDPR(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDPR(bp));

    /* Case 1 */
    if (pre_alloc && next_alloc) {
        return bp;
    }

    /* Case 2 */
    else if (pre_alloc && !next_alloc) {
        size += GET_SIZE(HDPR(NEXT_BLKP(bp)));
        PUT(HDPR(bp), PACK(size, 0));
        PUT(FTPR(bp), PACK(size, 0));
    }

    /* Case 3 */
    else if (!pre_alloc && next_alloc) {
        size += GET_SIZE(HDPR(PREV_BLKP(bp)));
        PUT(HDPR(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTPR(bp), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    /* Case 4 */
    else {
        size += GET_SIZE(HDPR(PREV_BLKP(bp))) + GET_SIZE(HDPR(NEXT_BLKP(bp)));
        PUT(HDPR(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTPR(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    return bp;
}


/*
 * first_fit - Find the first fit for a block with size bytes
 */
static void *first_fit(size_t size) {
    void *bp;
    for (bp = heap_listp; GET_SIZE(HDPR(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDPR(bp)) && size <= GET_SIZE(HDPR(bp))) {
            return bp;
        }
    }
    return NULL;
}


/*
 * place - Place block of size bytes at start of free block bp
 *         and split if the remainder would be at least minimum block size
 */
static void place(void *bp, size_t size) {
    size_t bsize = GET_SIZE(HDPR(bp));
    size_t leftSize = bsize - size;

    if (leftSize < 2*DSIZE) {
        PUT(HDPR(bp), PACK(bsize, 1));
        PUT(FTPR(bp), PACK(bsize, 1));
    } else {
        PUT(HDPR(bp), PACK(size, 1));
        PUT(FTPR(bp), PACK(size, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDPR(bp), PACK(leftSize, 0));
        PUT(FTPR(bp), PACK(leftSize, 0));
    }
}















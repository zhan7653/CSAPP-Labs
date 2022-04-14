/*
 * Implicit free list on the textbook.
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
#define PUT(p, val) (*(unsigned int *)(p) = (unsigned int)(val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDPR(bp) ((char *)(bp) - WSIZE)
#define FTPR(bp) ((char *)(bp) + GET_SIZE(HDPR(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

/* Given block ptr bp, read and write its predecessor and successor field */
#define GET_PRED(bp)  ((char *)GET(bp))
#define GET_SUCC(bp)  ((char *)GET((char *)(bp) + WSIZE))
#define SET_PRED(bp, val)  (PUT(bp, val))
#define SET_SUCC(bp, val)  (PUT((char *)(bp) + WSIZE, val))


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
    if ((heap_listp = mem_sbrk(6*WSIZE)) == (void *)-1) {
        printf("mm_init fault!\n");
        return -1;
    }
    PUT(heap_listp, 0);                             // alignment padding
    PUT(heap_listp + (1*WSIZE), PACK(2*DSIZE, 1));    // prologue header
    PUT(heap_listp + (4*WSIZE), PACK(2*DSIZE, 1));    // prologue footer
    PUT(heap_listp + (5*WSIZE), PACK(0, 1));    // epilogue header
    heap_listp += (2*WSIZE);
    SET_PRED(heap_listp, heap_listp);
    SET_SUCC(heap_listp, heap_listp);

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
    SET_PRED(GET_SUCC(heap_listp), bp);
    SET_PRED(bp, heap_listp);
    SET_SUCC(bp, GET_SUCC(heap_listp));
    SET_SUCC(heap_listp, bp);    
    // coalesce(bp);
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
    SET_PRED(GET_SUCC(heap_listp), bp);
    SET_PRED(bp, heap_listp);
    SET_SUCC(bp, GET_SUCC(heap_listp));
    SET_SUCC(heap_listp, bp);
    PUT(HDPR(NEXT_BLKP(bp)), PACK(0, 1));


    // return coalesce(bp);
    return bp;
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
    char *bp = heap_listp;
    do {
        if (!GET_ALLOC(HDPR(bp)) && size <= GET_SIZE(HDPR(bp))) {
            return bp;
        }
        bp = (char *)GET_SUCC(bp);
    } while (bp != heap_listp);

    return NULL;
}


/*
 * place - Place block of size bytes at start of free block bp
 *         and split if the remainder would be at least minimum block size
 */
static void place(void *bp, size_t size) {
    size_t bsize = GET_SIZE(HDPR(bp));
    size_t leftSize = bsize - size;
    char *oldpred = GET_PRED(bp);
    char *oldsucc = GET_SUCC(bp);

    if (leftSize < 2*DSIZE) {
        SET_SUCC(oldpred, oldsucc);
        SET_PRED(oldsucc, oldpred);
        PUT(HDPR(bp), PACK(bsize, 1));
        PUT(FTPR(bp), PACK(bsize, 1));
    } else {
        PUT(HDPR(bp), PACK(size, 1));
        PUT(FTPR(bp), PACK(size, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDPR(bp), PACK(leftSize, 0));
        PUT(FTPR(bp), PACK(leftSize, 0));
        SET_PRED(bp, oldpred);
        SET_SUCC(bp, oldsucc);
        SET_PRED(oldsucc, bp);
        SET_SUCC(oldpred, bp);
        
    }
}















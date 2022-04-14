/*
 * Simple allocator based on explicit free lists, best-fit placement,
 * and boundary tag coalescing as described in the CS:APP3e text.
 * 
 * Blocks must be aligned to doubleword (8 byte) boundaries.
 * Minimum block size is 16 bytes(4 words).
 * 
 * Results for mm malloc:
 * trace  valid  util     ops      secs  Kops
 *  0       yes   99%    5694  0.000155 36783
 *  1       yes  100%    5848  0.000212 27520
 *  2       yes   99%    6648  0.000200 33207
 *  3       yes  100%    5380  0.000164 32865
 *  4       yes   99%   14400  0.000161 89552
 *  5       yes   96%    4800  0.002528  1899
 *  6       yes   95%    4800  0.002414  1988
 *  7       yes   55%   12000  0.030742   390
 *  8       yes   51%   24000  0.080844   297
 *  9       yes  100%   14401  0.000193 74655
 * 10       yes   87%   14401  0.000121119411
 * Total          89%  112372  0.117734   954

 * Perf index = 54 (util) + 40 (thru) = 94/100
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

/*
 * If BEST_FIT defined use best-fit search, else use first-fit search 
 */
#define BEST_FIT

/* Basic constants and macros */
#define ALIGNMENT 8 // single word (4) or double word (8) alignment
#define WSIZE 4     // word and header/footer size (bytes)
#define DSIZE 8     // double word size (bytes)
#define CHUNKSIZE (1<<6)   // extend heap by this default amount (bytes), this size affects memory utility

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)  

/* Pack a size and allocate bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)  (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (unsigned int)(val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

/* Given block ptr bp, read and write its predecessor and successor field */
#define GET_PRED(bp)  ((char *)GET(bp))
#define GET_SUCC(bp)  ((char *)GET((char *)(bp) + WSIZE))
#define SET_PRED(bp, val)  (PUT(bp, val))
#define SET_SUCC(bp, val)  (PUT((char *)(bp) + WSIZE, val))


/* Global variables */
static char *heap_listp = 0;    // pointer to prologue block

/* Function prototypes for internal helper routines */
static int mm_check(void)__attribute__((unused));
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t size);
static void place(void *bp, size_t size);
static void delete_node(void *bp);
static void insert_node_FIFO(void *bp);




/* 
 * mm_init - Initialize the malloc package.
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

    /* Prologue is the header of explicit free list */
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
        asize = ALIGN(size + DSIZE);
    }

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL) {
        return NULL;
    }
    place(bp, asize);
    // mm_check();
    return bp;
    
}

/*
 * mm_free - Free a block and coalesce.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
    // mm_check();
}

/*
 * mm_realloc - Change the size of the memory block pointed to by bp.
 */
void *mm_realloc(void *bp, size_t newsize)
{
    void *newptr;
    size_t oldsize;
    void *next_bp;
    int leftsize;

    /* If bp is NULL, the call is equivalent to mm_malloc(size) */
    if (bp == NULL) {
        return mm_malloc(newsize);
    }

    /* If newsize is equal to zero, the call is equivalent to mm_free(bp) */
    if (newsize == 0) {
        mm_free(bp);
        return NULL;
    }

    /* Otherwise, need to realloc */

    /* Adjust block size to include overhead and alignment reqs */
    if (newsize <= DSIZE) {
        newsize = 2 *DSIZE;
    } else {
        newsize = ALIGN(newsize + DSIZE);
    }
    oldsize = GET_SIZE(HDRP(bp));
    next_bp = NEXT_BLKP(bp);
    leftsize = oldsize - newsize;
    
    /* If newsize <= oldsize, just return original block */
    if (leftsize >= 0) {
        return bp;
    } 
    /* If newsize > oldsize, try to coalesce when next block is a free block or epilogue block */
    if (!GET_ALLOC(HDRP(next_bp)) || !GET_SIZE(HDRP(next_bp))) {
        leftsize = oldsize + GET_SIZE(HDRP(next_bp)) - newsize;
        if (leftsize < 0) { // still not enough after coalesce, so extend heap
            if (extend_heap(MAX(-leftsize, CHUNKSIZE) / WSIZE) == NULL) { // extend_heap also called coalesce, so there is only one free block
                return NULL;
            }
            leftsize += MAX(-leftsize, CHUNKSIZE);
        }

        /* Now we have enough space after the original block. */
        delete_node(next_bp);
        PUT(HDRP(bp), PACK(newsize + leftsize, 1));
        PUT(FTRP(bp), PACK(newsize + leftsize, 1));
        return bp;
    } 
    /* If can't coalesce next block, just call malloc to find a suitable block. */
    else {
        newptr = mm_malloc(newsize - DSIZE);  // malloc also add overhead to size, so need to minus DSIZE here
        memcpy(newptr, bp, oldsize - DSIZE);  // minus overhead to get payload size
        mm_free(bp);
    }

    return newptr;
}

/* 
 * mm_check - Simple check of the heap.
 */
static int mm_check(void) {
    char *bp;
    char *listp;

    /* Check all regular blocks */
    for (bp = NEXT_BLKP(heap_listp); GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        size_t alloc = GET_ALLOC(HDRP(bp));
        size_t alloc_next = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
        
        if (!alloc) {
            /* Are there any contiguous free blocks that somehow escaped coalescing? */
            if (!alloc_next) {
                printf("block escapes coalescing at %p while next block at %p\n", bp, NEXT_BLKP(bp));
                return -1;
            }

            /* Is every free block actually in the free list? */
            int flag = 1;
            for (listp = GET_SUCC(heap_listp); listp != heap_listp; listp = GET_SUCC(listp)) {
                if (bp == listp) {
                    flag = 0;
                    break;
                }
            }
            if (flag) {
                printf("block doesn't exist in free list at %p\n", bp);
                return -2;
            }
        }
    }
    return 0;
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
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);

}


/*
 * coalesce - Boundry tag coalescing. Return ptr to coalesced block.
 */
static void *coalesce(void *bp) {
    size_t pre_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    char *pre_bp;
    char *next_bp;

    /* Case 1 */
    if (pre_alloc && next_alloc) {
        insert_node_FIFO(bp);
        return bp;
    }

    /* Case 2 */
    else if (pre_alloc && !next_alloc) {
        next_bp = NEXT_BLKP(bp);
        delete_node(next_bp);
        size += GET_SIZE(HDRP(next_bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));     
    }

    /* Case 3 */
    else if (!pre_alloc && next_alloc) {
        pre_bp = PREV_BLKP(bp);
        delete_node(pre_bp);
        size += GET_SIZE(HDRP(pre_bp));       
        PUT(HDRP(pre_bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        bp = pre_bp;
    }

    /* Case 4 */
    else {
        pre_bp = PREV_BLKP(bp);
        delete_node(pre_bp);        
        next_bp = NEXT_BLKP(bp);
        delete_node(next_bp);        
        size += GET_SIZE(HDRP(pre_bp)) + GET_SIZE(HDRP(next_bp));
        PUT(HDRP(pre_bp), PACK(size, 0));
        PUT(FTRP(next_bp), PACK(size, 0));
        bp = pre_bp;
    }


    insert_node_FIFO(bp); 
    return bp;
}


/* 
 * find_fit - Find a fit for a block with size bytes .
 */
static void *find_fit(size_t size) {
#ifdef BEST_FIT
    /* Best fit search */
    char *bp = GET_SUCC(heap_listp);
    char *min_bp = NULL;
    size_t bsize;
    size_t min_size = 0x3fffffff; // INF

    while (bp != heap_listp) {
        bsize = GET_SIZE(HDRP(bp));
        if (size <= bsize) {
            /* Keep track of minsize free block */
            if (bsize < min_size) {
                min_bp = bp;
                min_size = GET_SIZE(HDRP(min_bp));
            }
        }
        bp = GET_SUCC(bp);
    }

    return min_bp;
#else
    /* First-fit search */
    char *bp = GET_SUCC(heap_listp);

    while (bp != heap_listp) {
        if (size <= GET_SIZE(HDRP(bp))) {
            return bp;
        }
        bp = GET_SUCC(bp);
    }

    return NULL; // No fit
#endif
}




/*
 * place - Place block of size bytes at start of free block bp
 *         and split if the remainder would be at least minimum block size
 */
static void place(void *bp, size_t size) {
    size_t bsize = GET_SIZE(HDRP(bp));
    size_t leftsize = bsize - size;

    if (leftsize < 2*DSIZE) {
        /* Not split, just allocate the free block and delete it from the free list */
        delete_node(bp);
        PUT(HDRP(bp), PACK(bsize, 1));
        PUT(FTRP(bp), PACK(bsize, 1));
    } else {
        /* Split, save the pred and succ field for the splited free block */
        char *oldpred = GET_PRED(bp);
        char *oldsucc = GET_SUCC(bp); 
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(leftsize, 0));
        PUT(FTRP(bp), PACK(leftsize, 0));
        /* Change address on free list to the new free block */
        SET_PRED(bp, oldpred);
        SET_SUCC(bp, oldsucc);
        SET_PRED(oldsucc, bp);
        SET_SUCC(oldpred, bp);  
        
    }
}

static void delete_node(void *bp) {
    char *oldpred = GET_PRED(bp);
    char *oldsucc = GET_SUCC(bp);
    SET_PRED(oldsucc, oldpred);
    SET_SUCC(oldpred, oldsucc);
}

static void insert_node_FIFO(void *bp) {
    SET_PRED(GET_SUCC(heap_listp), bp);
    SET_PRED(bp, heap_listp);
    SET_SUCC(bp, GET_SUCC(heap_listp));
    SET_SUCC(heap_listp, bp);    
}














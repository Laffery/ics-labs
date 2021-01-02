/*
 * mm.c - a simple allocator based on an explicit free list
 * 
 * name: Ao Yuchen
 * s_id: 518021910545
 * 
 * The explicit free list is used to organize blocks. 
 * Each block has a header and a footer, containing info of size and allocation.
 * Besides, each free block has 8 byte predecessor pointer placed right after
 * header, followed by a successor pointer, which stores the previous and the 
 * next free block in the free list separately. Those directed pointers form a
 * double linked list as the free list.
 * 
 * The allocator is aligned to 8, using LIFO-ordering, first-fit and immediate 
 * coalescing, and its minimum block size is 6 words due to the boundary tags 
 * and pointers.
 *
 * The allocator initializes the heap by putting a padding word and a 
 * prologue block at the beginning and a epilogue block at the end, 
 * then put a free chunk between.
 * 
 * On allocation, the allocator first selects a free block in the free list 
 * using first-fit, split it if necessary, then set the selected block to 
 * allocated by changing its header and footer.
 * 
 * If the allocator split the block, it will coalesce the rest part if necessary, 
 * then add it to the beginning of the free list.
 * 
 * To free a block, the allocator simply set its tags and pointers, then coalesce
 * it and add it to the beginning of the free list.
 * 
 * The allocator will extend the heap by calling mem_sbrk, adding a new free block 
 * to the free list every time it finds there is no free block big enough.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* TextBook */
/* Basic constants and macros */
#define WSIZE       4           /* Word and header/footer size (bytes) */
#define DSIZE       8           /* Double word size (bytes) */
#define CHUNKSIZE   (1 << 12)   /* Extend heap by this amount (bytes) */

#define MAX(x, y)   ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)   ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(unsigned int *)(p))
#define PUT(p, val)     (*(unsigned int *)(p) = (val))

/* Read and write an address(pointer) at address p */
#define _GET_(p)        (*(unsigned long *)(p))
#define _PUT_(p, val)   (*(unsigned long *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp of a free block */
#define PRED(bp)        ((char *)(bp))          /* addr of pred ptr */
#define SUCC(bp)        ((char *)(bp) + DSIZE)  /* addr of succ ptr */

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)   ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)   ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Given block ptr bp of a free block */
#define NEXT_FREE(bp)   ((char*)_GET_(SUCC(bp)))    /* addr of next free block */
#define PREV_FREE(bp)   ((char*)_GET_(PRED(bp)))    /* addr of prev free block */

/* Some Global variables */
static char* heap_listp;            /* points to the prologue block */
static char* free_listp = NULL;     /* points to the first free block */
static int flag;                    /* 1 means have free blocks */

/* 
 **********************************************
 * two funcs on free list: ins_free/rmv_free * 
 ********************************************** 
 */

/*
 * ins_free - add a free block to the top of the free list.
 *      Using LIFO, free a blocks costs const-time.
 */
static void ins_free(void *bp)
{
    /* bp is on top, whose pred is null */
    _PUT_(PRED(bp), 0);

    if (free_listp != NULL) {
        _PUT_(PRED(free_listp), (unsigned long)bp);
        _PUT_(SUCC(bp), (unsigned long)free_listp);
    }

    else 
        _PUT_(SUCC(bp), 0);
    
    free_listp = bp;
}

/*
 * rmv_free - remove a free block from the free list.
 *      two cases:
 *          1. it's allocated
 *          2. it's coalesced
 */
static void rmv_free(void *bp)
{
    /* prev/next free blocks of bp */
    char* prev = PREV_FREE(bp);
    char* next = NEXT_FREE(bp);

    if (prev && next) {             /* case 1 */ 
        _PUT_(PRED(next), _GET_(PRED(bp)));
        _PUT_(SUCC(prev), _GET_(SUCC(bp)));
    }

    else if (prev && !next)         /* case 2 */
        _PUT_(SUCC(prev), 0);

    else if (!prev && next) {       /* case 3 */
        _PUT_(PRED(next), 0);
        free_listp = next;
    }

    else                            /* case 4 */
        free_listp = NULL;

    /* set pred/succ ptr of bp to nullptr in case of reuse */
    _PUT_(PRED(bp), 0);
    _PUT_(SUCC(bp), 0);
}

/*
 * coalesce - combine free block return the block ptr of the combined block.
 *     four cases:
 *         prev alloc, next alloc;
 *         prev free , next alloc;
 *         prev alloc, next free ;
 *         prev free , next free ;
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));


    if (prev_alloc && next_alloc)               /* Case 1 */
        ins_free(bp);

    else if (prev_alloc && !next_alloc) {       /* Case 2 */
        rmv_free(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        ins_free(bp);
    }

    else if (!prev_alloc && next_alloc) {       /* Case 3 */
        rmv_free(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        ins_free(bp);
    }

    else {                                      /* Case 4 */
        rmv_free(PREV_BLKP(bp));
        rmv_free(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        ins_free(bp);
    }
    
    return bp;
}


/*
 * extend_heap - extend heap space.
 *      called in two situations:
 *          1. initialize heap
 *          2. can't find a fit 
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footerand the epilogue header  */
    PUT(HDRP(bp), PACK(size, 0));           /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));           /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   /* New epilogue header */
    
    /* Initial pred ptr and succ ptr */
    _PUT_(PRED(bp), 0);                     /* Predecessor pointer */
    _PUT_(SUCC(bp), 0);                     /* Successor pointer */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void *) -1)
        return -1;

    PUT(heap_listp, 0);                             /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));    /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));        /* Epilogue header */
    heap_listp += (2*WSIZE);
    
    /* Set global variables */
    free_listp = NULL;
    flag = 0;

    /* Extend the empty head with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;

    return 0;
}


/*
 * find_fit - to find a fit block in heap.
 *      take first fit strategy.
 */
static void *find_fit(size_t asize)
{
    void *bp = free_listp;

    while (bp != NULL) {   
        if (GET_SIZE(HDRP(bp)) >= asize) /* space enough */
            if (!flag || GET_SIZE(HDRP(NEXT_BLKP(bp))) != 0)
                return bp; /* first fit */

        bp = NEXT_FREE(bp);
    }

    return NULL; /* not fit */
}

/*
 * place - place asize bytes at address bp.
 *      only if rest size is greater than size of minimum block,
 *  we should make rest block with enough size free.
 */
static void place(void *bp, size_t asize)
{
    /* The size of the free block to be allocated */
    size_t size = GET_SIZE(HDRP(bp));          

    /* If the size of the rest is greater than the minimum block size */
    if ((size - asize) >= (3*DSIZE)) {
        rmv_free(bp);
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        /* rest part, add to free list */
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(size - asize, 0));
        PUT(FTRP(bp), PACK(size - asize, 0));
        ins_free(bp);
    }

    else {
        rmv_free(bp);
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
    }
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
*/
void *mm_malloc(size_t size)
{
    size_t asize;        /* Adjusted block size */
    size_t extendsize;   /* Amount to extend heap if no fit */
    char* bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /*
     * Adjust block size to include overhead
     * (header, footer and two pointers, 6 words in total) 
     * and alignment reqs */
    if (size <= DSIZE)
        asize = 3*DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - free a block and uses boundary-tag coalesce it.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    _PUT_(PRED(ptr), 0);
    _PUT_(SUCC(ptr), 0);
    coalesce(ptr);
}

/*
 * mm_realloc - realloc the allocated block to block with size bytes. 
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    if (!flag) {
        /* free list is empty, malloc directly cause space is enough */
        newptr = mm_malloc(size);
        if (newptr == NULL)
            return NULL;

        copySize = *(size_t *)((char *)oldptr - DSIZE);
        if (copySize > size)
            copySize = size;

        memcpy(newptr, oldptr, copySize);
        mm_free(oldptr);
        flag = 1;

        return newptr;
    }
    else
    {
        char* free_1st = NEXT_BLKP(oldptr);
        size_t diff = DSIZE * ((size - GET_SIZE(HDRP(oldptr)) + (2*DSIZE-1)) / DSIZE);
        
        /* space not enough, extend heap */
        if (GET_SIZE(HDRP(free_1st)) < (diff + 3*DSIZE))
            free_1st = extend_heap(CHUNKSIZE/WSIZE);

        rmv_free(free_1st);
        free_1st += diff;

        PUT(HDRP(free_1st), PACK(GET_SIZE(HDRP(free_1st-diff))-diff, 0));
        PUT(FTRP(free_1st), PACK(GET_SIZE(HDRP(free_1st-diff))-diff, 0));

        _PUT_(PRED(free_1st), 0);
        _PUT_(SUCC(free_1st), 0);

        PUT(HDRP(oldptr), PACK(GET_SIZE(HDRP(oldptr))+diff, 1));
        PUT(FTRP(oldptr), PACK(GET_SIZE(HDRP(oldptr)), 1));

        ins_free(free_1st);
        return oldptr;
    }
}


/*
 * mm_check - a heap consistency checker accorind to pdf.
 */
int mm_check(void)
{
    char *bp; /* heap_listp */
    char *fp; /* free_listp */
    
    /* Is every block in the free list marked as free? */
    for (fp = free_listp; fp != NULL; fp = NEXT_FREE(fp)) {
        if (GET_ALLOC(fp)) {
            fprintf(stderr, "Some block in the free list is not marked as free.\n");
            return 0;
        }
    }

    /* Are there any contigious free blocks that somehow escaped coalescing? */
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!(GET_ALLOC(HDRP(bp)) || GET_ALLOC(HDRP(NEXT_BLKP(bp))))) {
            fprintf(stderr, "There are some contagious free blocks.\n");
            return 0;
        }
    }

    int count = 0; /* count of free blocks */
    /* Is every free block actually in the free list? */
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
        if(!GET_ALLOC(HDRP(bp)))
            count++;

    for (fp = free_listp; fp != NULL; fp = NEXT_FREE(fp))
        count--;

    if (count) {
        fprintf(stderr, "Some free block are not in the free list.\n");
        return 0;
    }


    for (fp = free_listp; fp != NULL; fp = NEXT_FREE(fp)) {
        /* Do the pointers in the free list point to valid free blocks? */
        if (_GET_(PRED(free_listp)) != 0) {
            fprintf(stderr, "Some ptrs in the free list are invalid.\n");
            return 0;
        }

        /* Do any allocated blocks overlap? */
        if (_GET_(PRED(fp)) == _GET_(SUCC(fp)) && _GET_(PRED(fp)) != 0) {
            fprintf(stderr, "Some allocated blocks overlap.\n");
            return 0;
        }

        /* Do the pointers in a heap block point to valid heap address? */
        if (_GET_(SUCC(fp)) != 0 && 
            (_GET_(SUCC(fp)) != (unsigned long)NEXT_FREE(fp) || 
            _GET_(PRED(NEXT_FREE(fp))) != (unsigned long)fp)) {
            fprintf(stderr, "Some ptrs in the heap block are invalid.\n");
            return 0;
        }
    }

    return 1;
}
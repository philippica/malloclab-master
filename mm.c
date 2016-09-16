/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>


#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "philippica",
    /* First member's full name */
    "Pu",
    /* First member's email address */
    "philippicapyj@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 
 * mm_init - initialize the malloc package.
 */


#define UINT                  unsigned int
#define WSIZE                 (sizeof(int))
#define DSIZE                 (2 * (WSIZE))
#define MALLOC_USED           1
#define MALLOC_FREE           0
#define MALLOC_PADDING        0
#define EXTEND_HEAP_SIZE      (1 << 10)
#define PUT(p, val)           (GET(p) = (val))
#define GET(p)                (*(unsigned int *)(p))
#define GET_SIZE(p)           (GET(p) & ~0x7)
#define HD(bp)                ((char*)(bp) - WSIZE)
#define FT(bp)                ((char*)(bp) + GET_SIZE(HD(bp)) - DSIZE)
#define PREV_BLKP(bp)         ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE))
#define NEXT_BLKP(bp)         ((char*)(bp) + GET_SIZE((char*)(bp) - WSIZE))
#define GET_ALLOC(bp)         ((*(unsigned int *)(bp)) & 1)
#define max(x,y) ((x) > (y) ? x : y)

void* coalesce(void *bp)
{
    int prev_alloc = GET_ALLOC(FT(PREV_BLKP(bp)));
    int next_alloc = GET_ALLOC(HD(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HD(bp));
    if(prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HD(NEXT_BLKP(bp)));
        PUT(HD(bp), size | 0);
        PUT(FT(bp), size | 0);
    }
    else if(!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HD(PREV_BLKP(bp)));
        PUT(FT(bp), size | 0);
        PUT(HD(PREV_BLKP(bp)), size | 0);
        return PREV_BLKP(bp);
    }
    else if(!prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HD(PREV_BLKP(bp))) + GET_SIZE(FT(NEXT_BLKP(bp)));
        PUT(FT(NEXT_BLKP(bp)), size | 0);
        PUT(HD(PREV_BLKP(bp)), size | 0);
        return PREV_BLKP(bp);
    }
	return bp;
}


void* extend_heap(size_t words)
{
	UINT *p;
	size_t size = (words % 2 == 0) ? words : words + 1; // The size must be an even
	size *= WSIZE;
	p = mem_sbrk(size);
	if((void*) -1 == p)
	{
		return NULL;
	}
	PUT(HD(p), size | MALLOC_FREE);
	PUT(FT(p), size | MALLOC_FREE);
	PUT(HD(NEXT_BLKP(p)), 0 | MALLOC_USED);
	return coalesce(p);
}


void* head;
int mm_init(void)
{
	head = mem_sbrk(4 * WSIZE);
	UINT* p = (UINT *)head;
	*p = MALLOC_PADDING;
	*(p + 1) = WSIZE * 2 | MALLOC_USED;    // Prologue header
	*(p + 2) = WSIZE * 2 | MALLOC_USED;    // Prologue footer
	*(p + 3) = 0         | MALLOC_USED;    // Epilogue header
	head = (void *)(p + 2);
	extend_heap(EXTEND_HEAP_SIZE / WSIZE);
	return 0;
}




void* find_fit(size_t size)
{
    void* bp;
    for(bp = head;GET_SIZE(HD(bp));bp = NEXT_BLKP(bp))
    {
        if(!GET_ALLOC(HD(bp)) && GET_SIZE(HD(bp)) >= size)return bp;
    }
    return NULL;
}

void place(void* bp, size_t size)
{
    size_t asize = GET_SIZE(HD(bp));
    if(asize - size <= DSIZE)
    {
        size = GET_SIZE(HD(bp));
       	PUT(HD(bp), size | MALLOC_USED);
        PUT(FT(bp), size | MALLOC_USED);
    }
    else
    {
       	PUT(HD(bp), size | MALLOC_USED);
        PUT(FT(bp), size | MALLOC_USED);
        PUT(FT(bp) + WSIZE, (asize - size) | MALLOC_FREE);
        PUT(FT(FT(bp) + DSIZE), (asize - size) | MALLOC_FREE);
    }
}


int cnt = 0;
void *mm_malloc(size_t size)
{
	cnt++;
	if(cnt == 76)
	{
		puts("1");
	}
    if(size == 0)return NULL;
    if(size <  DSIZE)
    {
        size = 2 * DSIZE;
    }
    else
    {
        size = DSIZE * ((size + 2 * DSIZE - 1) / DSIZE);
    }
    void* bp;
    if((bp = find_fit(size)) != NULL)
    {
        place(bp,size);
        return bp;
    }
    size_t extendsize = max(size, 2* WSIZE);
    bp = extend_heap(extendsize / WSIZE);
    place(bp, size);
    return bp;
}
void mm_free(void* bp)
{
	cnt++;
	if(cnt == 76)
	{
		puts("1");
	}
    int size = GET_SIZE(HD(bp));
    PUT(HD(bp), size | MALLOC_FREE);
    PUT(FT(bp), size | MALLOC_FREE);
    coalesce(bp);
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}















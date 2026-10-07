/*
 * mm-naive.c - 가장 빠르지만 메모리 효율이 가장 낮은 malloc 구현.
 * 
 * 이 단순한 방식은 brk 포인터를 늘리는 것만으로
 * 블록을 할당한다. 블록은 순수한 사용자 데이터로 구성되며
 * 헤더와 푸터가 없다. 블록을 합치거나 재사용하지 않는다.
 * realloc은 mm_malloc과 mm_free를 직접 사용하여 구현한다.
 * 
 * 학생 참고: 이 주석을 자신의 구현 방식을
 * 전체적으로 설명하는 주석으로 교체한다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*
 * 학생 참고: 다른 작업을 시작하기 전에
 * 아래 구조체에 팀 정보를 입력한다.
 */
team_t team = {
    /* 팀 이름 */
    "ateam",
    /* 첫 번째 구성원의 전체 이름 */
    "Harry Bovik",
    /* 첫 번째 구성원의 이메일 주소 */
    "bovik@cs.cmu.edu",
    /* 두 번째 구성원의 전체 이름(없으면 빈 문자열) */
    "",
    /* 두 번째 구성원의 이메일 주소(없으면 빈 문자열) */
    ""};

/* 기본 상수하고 매크로 */
#define ALIGNMENT 8

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

static char *heap_listp;

#define MAX(x, y) ((x) > (y) ? (x):(y))

#define PACK(size, alloc) ((size) | (alloc)) //크기와 사용 여부 합치기

//주소 값 가져오고 주소에 다른 값 입력하기
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

// 크기와 사용여부 분리
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

//header와 footer 주소 찾기
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

// 다음 and 이전 블록 주소 찾기
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* 가장 가까운 ALIGNMENT의 배수로 올림한다 */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

//인접한 빈 블록 합치기
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); //이전 블록 사용 여부
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp))); //다음 블록 사용 여부
    size_t size = GET_SIZE(HDRP(bp));   // 현재 블록의 크기

    if (prev_alloc && next_alloc)
        return bp;
    
    // 다음 블록과 합치기
    else if (prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    // 이전 블록과 합치기
    else if (!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    //앞뒤 모두 합치기
    else{
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

//힙 확장
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE; //요청 크기 바이트로 바꾸기(8배수 맞추기)

    //새로확보한 영영의 주소
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

//맞는 블록 찾기(fisrt-fit)
static void *find_fit(size_t size)
{
    void *bp = heap_listp;
    for (; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (size <= GET_SIZE(HDRP(bp))))
            return bp;
    }
    return NULL;
}

// 빈 블록에 할당 표시 및 공간 분할
static void place(void *bp, size_t size)
{
    size_t csize = GET_SIZE(HDRP(bp));  // 빈 블록의 크기 읽기

    if ((csize - size) >= (2*DSIZE))    //남는 공간으로 블록이 되면 분할
    {
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-size, 0));
        PUT(FTRP(bp), PACK(csize-size, 0));
    }
    else    // 남는 공간이 작으면 전체 할당
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}
/*
 * mm_init - malloc 구현을 초기화한다.
 */
int mm_init(void)
{
    //초기 16바이트 확보
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    
    PUT(heap_listp, 0);     //정렬용 패딩
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));    //프롤로그 블록 헤더
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    //프롤로그 블록 푸터
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));    //에필로그 헤더
    heap_listp += (2*WSIZE);    //bp 위치로 이동

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) //실제 사용할 빈 블록 확보
        return -1;

    return 0;
}

// mm_malloc - 적절한 빈 블록을 찾아 할당하고, 없으면 힙을 확장한다.
void *mm_malloc(size_t size)
{
    size_t asize;      // 헤더·푸터와 정렬을 반영한 블록 크기
    size_t extendsize; // 적절한 빈 블록이 없을 때 힙을 늘릴 크기
    char *bp;

    // 크기가 0인 요청은 무시한다
    if (size == 0)
        return NULL;

    // 헤더·푸터와 정렬 조건을 반영해 블록 크기를 조정한다
    if (size <= DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

    // 요청을 수용할 빈 블록을 찾는다
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    // 적절한 블록이 없으면 힙을 확장한 뒤 할당한다
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - 블록 해제 시 아무 작업도 하지 않는다.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp)); //전체 사이즈 일기

    //사용여부 0으로 만들기
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    coalesce(bp);
}


void *mm_realloc(void *bp, size_t size)
{
    if (bp == NULL) //빈 블록일때
        return mm_malloc(size);

    if (size == 0)  //사이즈가 0일떄
    {
        mm_free(bp);
        return NULL;
    }

    size_t osize = GET_SIZE((HDRP(bp)));
    size_t copySize = (osize - DSIZE);
    size_t asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    
    if (size > copySize)
    {
        void *newbp = mm_malloc(size);
        if (newbp == NULL) return NULL;

        memcpy(newbp, bp, copySize);
        mm_free(bp);
        return newbp;
    }
    else if(size == copySize)
    {   
        return bp;
    }
    else
    {
        size_t remain = osize - asize;
        if (remain >= (2*DSIZE))
        {
            PUT(HDRP(bp), PACK(asize, 1));
            PUT(FTRP(bp), PACK(asize, 1));

            void *newbp = NEXT_BLKP(bp);

            PUT(HDRP(newbp), PACK((remain), 0));
            PUT(FTRP(newbp), PACK((remain), 0));
            coalesce(newbp);
        }
        return bp;
    }
}

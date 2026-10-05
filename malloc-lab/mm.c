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

/* 단일 워드(4바이트) 또는 더블 워드(8바이트) 단위 정렬 */
#define ALIGNMENT 8

/* 가장 가까운 ALIGNMENT의 배수로 올림한다 */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*
 * mm_init - malloc 구현을 초기화한다.
 */
int mm_init(void)
{
    return 0;
}

/*
 * mm_malloc - brk 포인터를 늘려 블록을 할당한다.
 *     블록 크기는 항상 정렬 단위의 배수가 되도록 한다.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
        return NULL;
    else
    {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - 블록 해제 시 아무 작업도 하지 않는다.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - mm_malloc과 mm_free를 사용하여 단순하게 구현한다.
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
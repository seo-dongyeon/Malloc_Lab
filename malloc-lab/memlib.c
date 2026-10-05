/*
 * memlib.c - 메모리 시스템을 모사하는 모듈.
 *            학생이 만든 malloc 구현과 libc의 malloc 구현을
 *            번갈아 호출할 수 있도록 필요하다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#include "memlib.h"
#include "config.h"

/* 파일 내부에서 사용하는 변수 */
static char *mem_start_brk;  /* 힙의 첫 바이트를 가리킨다 */
static char *mem_brk;        /* 힙의 마지막 바이트를 가리킨다 */
static char *mem_max_addr;   /* 허용되는 최대 힙 주소 */ 

/*
 * mem_init - 모사할 메모리 시스템을 초기화한다.
 */
void mem_init(void)
{
    /* 사용 가능한 가상 메모리를 모사할 공간을 할당한다 */
    if ((mem_start_brk = (char *)malloc(MAX_HEAP)) == NULL) {
	fprintf(stderr, "mem_init_vm: malloc error\n");
	exit(1);
    }

    mem_max_addr = mem_start_brk + MAX_HEAP;  /* 허용되는 최대 힙 주소 */
    mem_brk = mem_start_brk;                  /* 처음에는 힙이 비어 있다 */
}

/*
 * mem_deinit - 메모리 시스템을 모사하는 데 사용한 공간을 해제한다.
 */
void mem_deinit(void)
{
    free(mem_start_brk);
}

/*
 * mem_reset_brk - 모사한 brk 포인터를 초기화하여 힙을 비운다.
 */
void mem_reset_brk()
{
    mem_brk = mem_start_brk;
}

/*
 * mem_sbrk - sbrk 함수를 단순하게 모사한다. 힙을
 *    incr 바이트만큼 늘리고 새 영역의 시작 주소를 반환한다.
 *    이 모형에서는 힙을 줄일 수 없다.
 */
void *mem_sbrk(int incr) 
{
    char *old_brk = mem_brk;

    if ( (incr < 0) || ((mem_brk + incr) > mem_max_addr)) {
	errno = ENOMEM;
	fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
	return (void *)-1;
    }
    mem_brk += incr;
    return (void *)old_brk;
}

/*
 * mem_heap_lo - 힙의 첫 바이트 주소를 반환한다.
 */
void *mem_heap_lo()
{
    return (void *)mem_start_brk;
}

/*
 * mem_heap_hi - 힙의 마지막 바이트 주소를 반환한다.
 */
void *mem_heap_hi()
{
    return (void *)(mem_brk - 1);
}

/*
 * mem_heapsize() - 힙의 크기를 바이트 단위로 반환한다.
 */
size_t mem_heapsize() 
{
    return (size_t)(mem_brk - mem_start_brk);
}

/*
 * mem_pagesize() - 시스템의 페이지 크기를 반환한다.
 */
size_t mem_pagesize()
{
    return (size_t)getpagesize();
}

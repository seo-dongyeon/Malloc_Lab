#include <stdio.h>

extern int mm_init (void);
extern void *mm_malloc (size_t size);
extern void mm_free (void *ptr);
extern void *mm_realloc(void *ptr, size_t size);


/*
 * 학생들은 1명 또는 2명으로 팀을 구성한다.
 * 팀 이름, 구성원의 이름과 로그인 ID를
 * bits.c 파일의 이 구조체에 입력한다.
 */
typedef struct {
    char *teamname; /* ID1+ID2 또는 ID1 */
    char *name1;    /* 첫 번째 구성원의 전체 이름 */
    char *id1;      /* 첫 번째 구성원의 로그인 ID */
    char *name2;    /* 두 번째 구성원의 전체 이름(있는 경우) */
    char *id2;      /* 두 번째 구성원의 로그인 ID(있는 경우) */
} team_t;

extern team_t team;


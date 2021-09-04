#ifndef __GLUETHREAD__
#define __GLUETHREAD__

/*
 * データを持たせない
 * データのメンバーにglthreadを持たせる様に使う
 * struct any_data_t {
 *	char data[16];
 *	glthread_t glue;
 * }
 */
typedef struct _glthread {
    /* 次のデータのポインタ*/
    struct _glthread *left;
    /* 前のデータのポインタ */
    struct _glthread *tight;
} gltread_t;


void glethread_add_next(glthread_t *base_glthread, glthread_t *new_glthread);

void glthread_add_before(glthead_t *base_glthread, glthread_t *new_glthread);

void glthread_add_last(glthread_t *base_glthread, glthread_t *new_glthread);

void remove_glthread(glthread_t *glthread);

void init_glthread(glthread_t *glthread);

/* 前も後ろもNULLかチェック */
#define IS_GLTHREAD_LIST_EMPTY(glthreadptr) \
    ((glthreadptr)->right == 0 && (glthreadptr)->left == 0)

/* fn_nameの関数をstatic inlineで定義する
 * 
 * glthred_t型のメンバを持つ構造体person_tがある
 * typedef struct _person{
 *  int age;
 *  int weight;
 *  glthread_t glthread;
 * } person_t;
 * GLTHREAD_TO_STRUCT(glthread_to_person, person_t, glthred);
 * みたいな使われ方
 *
 * glthread_to_person(person->glthread);
 * は
 * (char *)(glthreadptr)
 * -(マイナス)
 * (char *)&(((person_t *)0)->glthread))
 * | | | | | | | | | | | | | | | | | | | | | |
 * |       |       |               |
 *    age    weight    glthread
 *                 |ここからperson_tでglthreadまでの距離を引く
 * char *でキャストしているのはbyte単位で計算したいため(アドレスはbyte単位)
 * */
#define GLTHREAD_TO_STRUCT(fn_name, structure_name, field_name) \
    static inline structure_name *fn_name(glthread_t *glthreadptr){ \
	return (structure_name *)((char *)(glthreadptr) - (char *)&(((structure_name *)0)->field_name)); \
    }

#define BASE(glthreadptr) ((glthreadptr)->right)

/* イテレートのマクロ
 * BEGINとENDの間に簡単に処理を書ける
 * */
#define ITERATE_GLTHREAD_BEGIN(glthreadptrstart, glthreadadptr){ \
    glthread_t *_glthread_ptr = NULL; \
    glthreadptr = BASE(glthreadptrstart); \
    for(; glthreadptr != NULL; glthreadptr = _glthread_ptr){ \
	_glthreadptr = (glthreadptr)->right;
#define ITERATE_GLTHREAD_END(glthreadptrstart, glthreadptr) }}

/* offset引いたアドレスを返す 使う側でキャストする */
#define GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthreadptr, offset) \
    (void *)((char *)(glthreadptr) - offset)

void delete_glthread_list(glthread_t *base_glthread);

unsigned int delete_glthread_list_count(glthread_t *base_glthread);

void glthread_prioriry_insert(glthread_t *base_glthread, glthread_t *glthread, int (*comp_fn)(void *, void *), int offset);


#endif

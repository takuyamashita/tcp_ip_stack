#include <stdlib.h>

#include "glthread.h"

void init_glthread(glthread_t *glthread){

    glthread->left = NULL;
    glthread->right = NULL;
}

/* curr_glthreadの次にnew_glthreadを挿入し、リンクを繋ぎ直します */
void glthread_add_next(glthread_t *curr_glthread, glthread_t *new_glthread){   

    if(!curr_glthread->right){

	curr_glthread->right = new_glthread;
	new_glthread->left = curr_glthread;
	
	return;
    }
    
    /* rightに既に入っている場合はcurrとcurr->rightの間に入れる  */
    glthread_t *temp = curr_glthread->right;
    
    curr_glthread->right = new_glthread;
    new_glthread->left = curr_glthread;

    new_glthread->right = temp;
    temp->left = new_glthread;
}

/* curr_glthreadの前にnew_glthreadを挿入し、リンクを繋ぎ直します */
void glthread_add_before(glthread_t *curr_glthread, glthread_t *new_glthread){
    
    if(!curr_glthread->left){
	
	new_glthread->left = NULL;
	new_glthread->right = curr_glthread;

	curr_glthread->left = new_glthread;

	return;
    }

    glthread_t *temp = curr_glthread->left;

    temp->right = new_glthread;
    new_glthread->left = temp;

    new_glthread->right = curr_glthread;
    curr_glthread->left = new_glthread;
}

/* curr_glthreadを取り除き、繋ぎ直せるならリンクを繋ぎ直します */
void remove_glthread(glthread_t *curr_glthread){

    if(!curr_glthread->left){
	if(curr_glthread->right){
	    /*
	     * |N|c|g|
	     */
	    curr_glthread->right->left = NULL;
	    curr_glthread->right = NULL;

	    return;
	}
	return;
    }

    if(!curr_glthread->right){
	/*
	 * |g|c|N|
	 */
	curr_glthread->left->right = NULL;
	curr_glthread->left = NULL;

	return;
    }

    /*
     * |g|c|g|
     */
    curr_glthread->left->right = curr_glthread->right;
    curr_glthread->right->left = curr_glthread->left;
    
    curr_glthread->left = NULL;
    curr_glthread->right = NULL;
}

/* 先にあるglthreadのリンクを切ります */
void delete_glthread_list(glthread_t *base_glthread){
    
    glthread_t *glthreadptr = NULL;

    /* base_glthreadから右方向にに回していく */
    ITERATE_GLTHREAD_BEGIN(base_glthread, glthreadptr){
	/* glthreadptrはbase_glthread->right  (not null) */
	remove_glthread(glthreadptr);
    } ITERATE_GLTHREAD_END(base_glthread, glthreadptr);
}

/* listの最後にnew_glthreadを加えます */
void glthread_add_list(glthread_t *base_glthread, glthread_t *new_glthread){
    
    glthread_t *glthreadptr = NULL,
	    *prevglthreadptr = NULL;
    
    /* glthread->rightをprevglthreadに入れる(上書きしていく) */
    ITERATE_GLTHREAD_BEGIN(base_glthread, glthreadptr){
	prevglthreadptr = glthreadptr;
    } ITERATE_GLTHREAD_END(base_glthread, glthreadptr);

    /* base_glthreadの長さが1だと一周もしない */
    if(prevglthreadptr)
	glthread_add_next(prevglthreadptr, new_glthread);
    else
	glthread_add_next(base_glthread, new_glthread);

}

/* 先にいくつglthreadがあるのかを返します */
unsigned int get_glthread_list_count(glthread_t *base_glthread){
    
    unsigned int count = 0;
    glthread_t *glthreadptr = NULL;

    ITERATE_GLTHREAD_BEGIN(base_glthread, glthreadptr){
	count ++;
   } ITERATE_GLTHREAD_END(base_glthread, glthreadptr);

   return count;
}

/* データを基準にbase_glthreadの適切な位置にglthreadを挿入します */
void glthread_priority_insert(
    glthread_t *base_glthread,
    glthread_t *glthread,
    int (*comp_fn)(void *, void *),
    int offset
){
    
    glthread_t *curr = NULL,
	    *prev = NULL;
    
    init_glthread(glthread);

    /* 空ならbase_glthreadの次にglthreadを繋げて返す */
    if(IS_GLTHREAD_LIST_EMPTY(base_glthread)){
	
	glthread_add_next(base_glthread, glthread);
	return;
    }

    /* 1つの接続しかない場合 */
    if(base_glthread->right && !base_glthread->right->right){
	/* base_glthreadの次のデータと、ターゲット(glthread)のデータを比べて
	 * -1を返せばbase_glthreadの次の次に、
	 * それ以外はbase_glthreadの次に挿入する
	 */
	if(comp_fn(
	    GLTHREAD_GET_USER_DATA_FROM_OFFSET(base_glthread->right, offset),
	    GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthread, offset)
	) == -1){ 
	    glthread_add_next(base_glthread->right, glthread);
	} else {
	    glthread_add_next(base_glthread, glthread);
	}

	return;
    }
    

    /* 
     * データが先なら -1以外
     * データが前なら -1
     */

    /* glthread->rightと比較して次(の方)で無ければ
     * base_glthreadの次に入れる  */
    if(comp_fn(
	GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthread, offset),
	GLTHREAD_GET_USER_DATA_FROM_OFFSET(base_glthread->right, offset)
    ) == -1){
	glthread_add_next(base_glthread, glthread);
	return;
    }

    ITERATE_GLTHREAD_BEGIN(base_glthread, curr){

	/* currと比較して次ならprevに保存しておく */
	if(comp_fn(
	    GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthread, offset),
	    GLTHREAD_GET_USER_DATA_FROM_OFFSET(curr, offset)
	) != -1){
	    prev = curr;
	    continue;
	}

	if(!prev)
	    glthread_add_next(base_glthread, glthread);
	else
	    glthread_add_next(prev, glthread);
	
	return;

    } ITERATE_GLTHREAD_END(base_glthread, curr);
}



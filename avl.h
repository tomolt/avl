#ifndef AVL_H
#define AVL_H

#include <stdint.h>

typedef struct AVL AVL;

struct AVL {
	void *root;
};

void avl_init  (AVL *avl);
int  avl_lookup(AVL *avl, uintmax_t key, void **value);
int  avl_insert(AVL *avl, uintmax_t key, void  *value);
int  avl_delete(AVL *avl, uintmax_t key);
void avl_free  (AVL *avl);
int  avl_check (AVL *avl);
void avl_print (AVL *avl, void *file);

#endif

#ifndef AVL_H
#define AVL_H

#include <stdint.h>
#include <stdio.h> /* FILE */

typedef void *AVL;

int  avl_lookup(AVL *avl, uintmax_t key, void **value);
int  avl_insert(AVL *avl, uintmax_t key, void  *value);
int  avl_delete(AVL *avl, uintmax_t key);
void avl_free  (AVL *avl);
int  avl_check (AVL  avl);
void avl_graph (AVL  avl, FILE *file);

#endif

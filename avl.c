#include <stdlib.h>
#include <stdio.h>

#include "avl.h"

#define MAXDEPTH 64

#define PUSH(s,t,n,d) (s[(t)++]=(uintptr_t)n|d)
#define POP(s,t,n,d)  (d=s[--(t)]&1,n=(Node*)(s[t]^d))

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

typedef struct Node Node;

struct Node {
	uintmax_t key;
	void     *value;
	Node     *edges[2];
	int8_t    bal;
};

static void
rotate(Node **e, int d)
{
	Node **f, **g;
	Node *a, *b;

	a = *e;
	f = &a->edges[d];
	b = *f;
	g = &b->edges[!d];
	
	if (d) { /* CCW rotation */
		a->bal = a->bal - 1 - MAX(b->bal, 0);
		b->bal = b->bal - 1 + MIN(a->bal, 0);
	} else { /* CW rotation */
		a->bal = a->bal + 1 - MIN(b->bal, 0);
		b->bal = b->bal + 1 + MAX(a->bal, 0);
	}

	*e =  b;
	*f = *g;
	*g =  a;
}

static void
balance(Node **e)
{
	Node **f;
	Node *a, *b;
	int d;
	a = *e;
	d = a->bal > 0;
	f = &a->edges[d];
	b = *f;
	if (a->bal * b->bal < 0) {
		rotate(f, !d);
	}
	rotate(e, d);
}

static void
retrace(AVL *avl, Node *node, int grow,
	uintptr_t stack[], int *depth)
{
	Node *parent;
	int d, chg = 1;
	for (;;) {
		if (!*depth) {
			avl->root = node;
			break;
		}
		POP(stack, *depth, parent, d);
		parent->edges[d] = node;
		if (!chg) {
			break;
		}
		if (grow) {
			parent->bal += d ? 1 : -1;
		} else {
			parent->bal -= d ? 1 : -1;
		}
		if (parent->bal < -1 || parent->bal > 1) {
			balance(&parent);
		}
		node = parent;
		if (grow) {
			chg = node->bal != 0;
		} else {
			chg = node->bal == 0;
		}
	}
}

static Node *
path_to(Node *node, uintmax_t key, uintptr_t stack[], int *depth)
{
	int d;
	while (node && key != node->key) {
		d = key > node->key;
		PUSH(stack, *depth, node, d);
		node = node->edges[d];
	}
	return node;
}

void
avl_init(AVL *avl)
{
	avl->root = 0;
}

int
avl_lookup(AVL *avl, uintmax_t key, void **value)
{
	Node *node;
	
	node = avl->root;
	while (node) {
		if (key == node->key) {
			*value = node->value;
			return 1;
		}
		node = node->edges[key > node->key];
	}
	
	return 0;
}

int
avl_insert(AVL *avl, uintmax_t key, void *value)
{
	uintptr_t stack[MAXDEPTH];
	Node *node;
	int depth = 0;

	node = path_to(avl->root, key, stack, &depth);
	if (node) {
		node->value = value;
		return 0;
	}

	node = malloc(sizeof *node);
	if (!node) return -1;
	node->key = key;
	node->value = value;
	node->edges[0] = 0;
	node->edges[1] = 0;
	node->bal = 0;

	retrace(avl, node, 1, stack, &depth);
	return 1;
}

int
avl_delete(AVL *avl, uintmax_t key)
{
	uintptr_t stack[MAXDEPTH];
	Node *node, *target = NULL;
	int depth = 0, d;

	target = path_to(avl->root, key, stack, &depth);
	if (!target) {
		return 0;
	}
	d = target->bal > 0;
	PUSH(stack, depth, target, d);
	path_to(target->edges[d], key, stack, &depth);

	POP(stack, depth, node, d);
	target->key   = node->key;
	target->value = node->value;

	retrace(avl, node->edges[!d], 0, stack, &depth);
	free(node);
	return 1;
}

void
avl_free(AVL *avl)
{
	Node *stack[MAXDEPTH], *node;
	int depth = 0;

	stack[depth++] = avl->root;
	while (depth) {
		node = stack[--depth];
		if (node->edges[0]) {
			stack[depth++] = node->edges[0];
		}
		if (node->edges[1]) {
			stack[depth++] = node->edges[1];
		}
		free(node);
	}
	avl->root = 0;
}

static int
check_node(Node *node)
{
	int l, h;
	if (!node) return 0;
	l = check_node(node->edges[0]);
	if (l < 0) return l;
	h = check_node(node->edges[1]);
	if (h < 0) return h;
	if (node->bal != h - l) return -1;
	if (node->bal < -1) return -2;
	if (node->bal >  1) return -2;
	return MAX(l, h) + 1;
}

int
avl_check(AVL *avl)
{
	return check_node(avl->root);
}

static void
print_node(Node *node, int col, FILE *file)
{
	int i;
	fprintf(file, " %+d[%03ju]", node->bal, node->key);
	col += 8;
	if (node->edges[0]) {
		print_node(node->edges[0], col, file);
	}
	if (node->edges[1]) {
		putc('\n', file);
		for (i = 0; i < col; ++i) {
			putc(' ', file);
		}
		print_node(node->edges[1], col, file);
	}
}

void
avl_print(AVL *avl, void *file)
{
	if (avl->root) {
		print_node(avl->root, 0, file);
		putc('\n', file);
	}
}


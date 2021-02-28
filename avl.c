#include <stdlib.h>
#include <stdio.h>

#include "avl.h"

#define MAXDEPTH 64

#define NODE(e)   (e)
#define BALAN(e)  (NODE(e)->bal)
#define EDGE(n,b) ((n)->bal=(b),(n))

#define PUSH(s,t,d,e) (s[t++]=(uintptr_t)e|d)
#define POP(s,t,d,e)  (t--,d=s[t]&1,e=(Edge*)(s[t]^d))

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

typedef struct Node Node;
typedef Node       *Edge;

struct Node {
	uintmax_t key;
	void     *value;
	Edge      edges[2];
	int8_t    bal;
};

static void
rotate(Edge *e, int d)
{
	Edge *f, *g;
	Node *a, *b;
	int x, y;

	a = NODE(*e);
	x = BALAN(*e);
	f = &a->edges[d];
	b = NODE(*f);
	y = BALAN(*f);
	g = &b->edges[!d];
	
	if (d) { /* CCW rotation */
		x = x - 1 - MAX(y, 0);
		y = y - 1 + MIN(x, 0);
	} else { /* CW rotation */
		x = x + 1 - MIN(y, 0);
		y = y + 1 + MAX(x, 0);
	}

	*e = EDGE(b, y);
	*f = *g;
	*g = EDGE(a, x);
}

static void
balance(Edge *e)
{
	Edge *f;
	Node *a;
	int x, y;
	a = NODE(*e);
	x = BALAN(*e);
	f = &a->edges[x > 0];
	y = BALAN(*f);
	if (x * y < 0) {
		rotate(f, x < 0);
	}
	rotate(e, x > 0);
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
	Node *stack[MAXDEPTH];
	Node *node, *parent;
	int depth = 0, d;
	int first;

	node = avl->root;
	while (node) {
		stack[depth++] = node;
		if (key == node->key) {
			node->value = value;
			return 0;
		}
		d = key > node->key;
		node = node->edges[d];
	}

	node = malloc(sizeof *node);
	if (!node) return -1;
	node->key = key;
	node->value = value;
	node->edges[0] = 0;
	node->edges[1] = 0;
	node->bal = 0;
	first = 1;

	while (depth) {
		parent = stack[--depth];
		d = key > parent->key;
		parent->edges[d] = node;
		if (!first && !node->bal) goto done;
		parent->bal += d ? 1 : -1;
		if (parent->bal < -1 || parent->bal > 1) {
			balance(&parent);
		}
		node = parent;
		first = 0;
	}
	avl->root = node;
	
done:
	return 1;
}

int
avl_delete(AVL *avl, uintmax_t key)
{
	uintptr_t stack[MAXDEPTH];
	Node *node, *parent, *target = NULL, *child;
	int depth = 0, d;
	int first;

	node = avl->root;
	while (node) {
		if (key == node->key) {
			target = node;
			d = node->bal > 0;
		} else {
			d = key > node->key;
		}
		stack[depth++] = (uintptr_t) node | d;
		node = node->edges[d];
	}
	if (!target) return 0;

	d = stack[--depth] & 1;
	node = (Node *) (stack[depth] ^ d);
	target->key   = node->key;
	target->value = node->value;

	child = node->edges[!d];
	free(node);
	node = child;
	first = 1;

	while (depth) {
		d = stack[--depth] & 1;
		parent = (Node *) (stack[depth] ^ d);
		parent->edges[d] = node;
		if (!first && node->bal) goto done;
		parent->bal -= d ? 1 : -1;
		if (parent->bal < -1 || parent->bal > 1) {
			balance(&parent);
		}
		node = parent;
		first = 0;
	}
	avl->root = node;

done:
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
	avl_init(avl);
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


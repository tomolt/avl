#include <stdlib.h>
#include <stdio.h>

#include "avl.h"

#define MAXDEPTH 64

#define SHIFT     (8*sizeof(Edge)-3)
#define NODE(e)   ((Node*)((e)<<3))
#define BALAN(e)  ((int)((e)>>SHIFT))
#define EDGE(n,b) ((Edge)(n)>>3|(Edge)(b)<<SHIFT)

#define PUSH(s,t,d,e) (s[t++]=(uintptr_t)e|d)
#define POP(s,t,d,e)  (t--,d=s[t]&1,e=(Edge*)(s[t]^d))

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

typedef struct Node Node;
typedef intptr_t    Edge;

struct Node {
	uintmax_t key;
	void     *value;
	Edge      edges[2];
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
	Edge *edge;
	Node *node;
	int d;
	
	edge = &avl->root;
	while (*edge) {
		node = NODE(*edge);
		if (key == node->key) {
			*value = node->value;
			return 1;
		}
		d = key > node->key;
		edge = &node->edges[d];
	}
	
	return 0;
}

int
avl_insert(AVL *avl, uintmax_t key, void *value)
{
	uintptr_t stack[MAXDEPTH];
	Edge *edge;
	Node *node;
	int depth = 0, d;
	int balan;

	edge = &avl->root;
	while (*edge) {
		node = NODE(*edge);
		if (key == node->key) {
			node->value = value;
			return 0;
		}
		d = key > node->key;
		PUSH(stack, depth, d, edge);
		edge = &node->edges[d];
	}

	node = malloc(sizeof *node);
	if (!node) return -1;
	node->key = key;
	node->value = value;
	node->edges[0] = 0;
	node->edges[1] = 0;
	*edge = EDGE(node, 0);

	while (depth) {
		POP(stack, depth, d, edge);
		balan = BALAN(*edge);
		balan += d ? 1 : -1;
		*edge = EDGE(NODE(*edge), balan);
		if (balan < -1 || balan > 1) {
			balance(edge);
		}
		balan = BALAN(*edge);
		if (!balan) break;
	}
	
	return 1;
}

int
avl_delete(AVL *avl, uintmax_t key)
{
	uintptr_t stack[MAXDEPTH];
	Edge *edge;
	Node *node, *target = NULL;
	int depth = 0, d;
	int balan;

	edge = &avl->root;
	while (*edge) {
		node = NODE(*edge);
		if (key == node->key) {
			target = node;
			d = BALAN(*edge) > 0;
		} else {
			d = key > node->key;
		}
		PUSH(stack, depth, d, edge);
		edge = &node->edges[d];
	}

	if (!target) return 0;
	target->key   = node->key;
	target->value = node->value;
	POP(stack, depth, d, edge);
	*edge = node->edges[!d];
	free(node);

	while (depth) {
		POP(stack, depth, d, edge);
		balan = BALAN(*edge);
		balan -= d ? 1 : -1;
		*edge = EDGE(NODE(*edge), balan);
		if (balan < -1 || balan > 1) {
			balance(edge);
		}
		balan = BALAN(*edge);
		if (balan) break;
	}

	return 1;
}

void
avl_free(AVL *avl)
{
	Edge stack[MAXDEPTH];
	Edge edge;
	Node *node;
	int depth = 0;

	stack[depth++] = avl->root;
	while (depth) {
		edge = stack[--depth];
		node = NODE(edge);
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
check_edge(Edge edge)
{
	Node *node;
	int bal, l, h;
	if (!edge) return 0;
	node = NODE(edge);
	bal  = BALAN(edge);
	l = check_edge(node->edges[0]);
	if (l < 0) return l;
	h = check_edge(node->edges[1]);
	if (h < 0) return h;
	if (bal != h - l) return -1;
	if (bal < -1 || bal > 1) return -2;
	return MAX(l, h) + 1;
}

int
avl_check(AVL *avl)
{
	return check_edge(avl->root);
}

static void
print_edge(Edge edge, int col, FILE *file)
{
	Node *node;
	
	node = NODE(edge);
	fprintf(file, " %+d[%03ju]", BALAN(edge), node->key);
	col += 8;

	if (node->edges[0]) {
		print_edge(node->edges[0], col, file);
	}

	if (node->edges[1]) {
		putc('\n', file);
		for (int i = 0; i < col; ++i) {
			putc(' ', file);
		}

		print_edge(node->edges[1], col, file);
	}
}

void
avl_print(AVL *avl, void *file)
{
	if (avl->root) {
		print_edge(avl->root, 0, file);
		putc('\n', file);
	}
}


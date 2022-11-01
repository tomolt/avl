#include <stdlib.h>
#include <stdio.h>

#include "avl.h"

#define MAXDEPTH 64

#define PUSH(s,t,n,d) (s[(t)++]=(uintptr_t)n|d)
#define POP(s,t,n,d)  (d=s[--(t)]&1,n=(Node*)(s[t]^d))

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

typedef struct Node Node;
typedef Node *Edge;

struct Node {
	uintmax_t key;
	void     *value;
	Edge      edges[2];
	int8_t    balance;
};

static void
rotate(Edge *edge, int direction)
{
	Edge *pivotEdge, *otherEdge;
	Node *pivotNode, *childNode;

	pivotNode = *edge;
	pivotEdge = &pivotNode->edges[direction];
	childNode = *pivotEdge;
	otherEdge = &childNode->edges[!direction];
	
	if (direction) { /* CCW rotation */
		pivotNode->balance = pivotNode->balance - 1 - MAX(childNode->balance, 0);
		childNode->balance = childNode->balance - 1 + MIN(pivotNode->balance, 0);
	} else { /* CW rotation */
		pivotNode->balance = pivotNode->balance + 1 - MIN(childNode->balance, 0);
		childNode->balance = childNode->balance + 1 + MAX(pivotNode->balance, 0);
	}

	*edge      =  childNode;
	*pivotEdge = *otherEdge;
	*otherEdge =  pivotNode;
}

static void
balance(Edge *edge)
{
	Node **f;
	Node *a, *b;
	int direction;
	a = *edge;
	direction = a->balance > 0;
	f = &a->edges[direction];
	b = *f;
	if (a->balance * b->balance < 0) {
		rotate(f, !direction);
	}
	rotate(edge, direction);
}

static void
retrace(AVL *avl, Node *node, int grow,
	uintptr_t stack[], int *depth)
{
	Node *parent;
	int direction, changed = 1;
	for (;;) {
		if (!*depth) {
			avl->root = node;
			break;
		}
		POP(stack, *depth, parent, direction);
		parent->edges[direction] = node;
		if (!changed) {
			break;
		}
		if (grow) {
			parent->balance += direction ? 1 : -1;
		} else {
			parent->balance -= direction ? 1 : -1;
		}
		if (parent->balance < -1 || parent->balance > 1) {
			balance(&parent);
		}
		node = parent;
		if (grow) {
			changed = node->balance != 0;
		} else {
			changed = node->balance == 0;
		}
	}
}

static Node *
path_to(Node *node, uintmax_t key, uintptr_t stack[], int *depth)
{
	int direction;
	while (node && key != node->key) {
		direction = key > node->key;
		PUSH(stack, *depth, node, direction);
		node = node->edges[direction];
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
	node->balance = 0;

	retrace(avl, node, 1, stack, &depth);
	return 1;
}

int
avl_delete(AVL *avl, uintmax_t key)
{
	uintptr_t stack[MAXDEPTH];
	Node *node, *target = NULL;
	int depth = 0, direction;

	target = path_to(avl->root, key, stack, &depth);
	if (!target) {
		return 0;
	}
	direction = target->balance > 0;
	PUSH(stack, depth, target, direction);
	path_to(target->edges[direction], key, stack, &depth);

	POP(stack, depth, node, direction);
	target->key   = node->key;
	target->value = node->value;

	retrace(avl, node->edges[!direction], 0, stack, &depth);
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
	int height0, height1;
	if (!node) return 0;
	height0 = check_node(node->edges[0]);
	if (height0 < 0) return height0;
	height1 = check_node(node->edges[1]);
	if (height1 < 0) return height1;
	if (node->balance != height1 - height0) return -1;
	if (node->balance < -1) return -2;
	if (node->balance >  1) return -2;
	return MAX(height0, height1) + 1;
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
	fprintf(file, " %+d[%03ju]", node->balance, node->key);
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


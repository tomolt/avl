#include <stdint.h>
#include <stdlib.h>

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
typedef intptr_t Edge;
typedef void *AVL;

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

int
avl_lookup(AVL *avl, uintmax_t key, void **value)
{
	Edge *edge;
	Node *node;
	int d;
	
	edge = (Edge *) avl;
	while (*edge) {
		node = NODE(*edge);
		if (key == node->key) {
			*value = node->value;
			return 0;
		}
		d = key > node->key;
		edge = &node->edges[d];
	}
	
	return -1;
}

int
avl_insert(AVL *avl, uintmax_t key, void *value)
{
	uintptr_t stack[MAXDEPTH];
	Edge *edge;
	Node *node;
	int depth = 0, d;
	int balan;

	edge = (Edge *) avl;
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
		if (!balan) break;
	}
	
	return 0;
}

#include <stdio.h>

static void
graph_rec(Edge edge, int col, FILE *file)
{
	Node *node;
	
	node = NODE(edge);
	fprintf(file, " %+d[%03ju]", BALAN(edge), node->key);
	col += 8;

	if (node->edges[0]) {
		graph_rec(node->edges[0], col, file);
	}

	if (node->edges[1]) {
		putc('\n', file);
		for (int i = 0; i < col; ++i) {
			putc(' ', file);
		}

		graph_rec(node->edges[1], col, file);
	}
}

static void
avl_graph(AVL avl, FILE *file)
{
	if (avl) {
		graph_rec((Edge) avl, 0, file);
		putc('\n', file);
	}
}

int
main()
{
	AVL avl = NULL;
	int i;
	srand(0);
	for (i = 0; i < 100; i++) {
		avl_insert(&avl, rand() % 1000, NULL);
	}
	avl_graph(avl, stdout);
	return 0;
}


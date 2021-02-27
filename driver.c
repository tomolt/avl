#include <stdlib.h>
#include <stdio.h>

#include "avl.h"

static AVL Tree;

int
main()
{
	uintmax_t key;
	int i;

	Tree = NULL;
	srand(0);
	for (i = 0; i < 100; i++) {
		key = rand() % 1000;
		printf("#%d INSERT [%03ju]\n", i, key);
		avl_insert(&Tree, key, NULL);
		avl_graph(Tree, stdout);
		if (avl_check(Tree) < 0) {
			printf("INCONSISTENCY\n");
			break;
		}
	}
	avl_free(&Tree);
	return 0;
}


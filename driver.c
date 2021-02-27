#include <stdlib.h>
#include <stdio.h>

#include "avl.h"

#define MAX_KEY 1000

typedef struct {
	uint8_t   *bits;
	size_t     num;
} Ref;

static void
ref_init(Ref *ref)
{
	ref->bits = calloc((MAX_KEY + 7) / 8, 1);
	ref->num  = 0;
}

static int
ref_lookup(Ref *ref, uintmax_t key)
{
	return (ref->bits[key / 8] >> (key % 8)) & 1;
}

static int
ref_insert(Ref *ref, uintmax_t key)
{
	if (ref_lookup(ref, key)) return 0;
	ref->bits[key / 8] |= 1 << (key % 8);
	ref->num++;
	return 1;
}

static int
ref_delete(Ref *ref, uintmax_t key)
{
	if (!ref_lookup(ref, key)) return 0;
	ref->bits[key / 8] &= ~(1 << (key % 8));
	ref->num--;
	return 1;
}

static void
ref_free(Ref *ref)
{
	free(ref->bits);
}

/* robust modulo */
static size_t
robmod(size_t n, size_t m)
{
	return m ? n % m : 0;
}

static AVL _avl;
static Ref _ref;

static void
drive(void)
{
	uintmax_t key;
	size_t run, counter;
	void *tmp;
	int a, b;

	_avl = 0;
	ref_init(&_ref);
	srand(0);
	
	printf("#IT\t#EL\n");
	
	counter = 0;
	for (;;) {
		switch (rand() % 4) {
		case 0: /* insert multiple */
			run = robmod(rand(), MAX_KEY - _ref.num);
			while (run--) {
				key = rand() % MAX_KEY;
				a = avl_insert(&_avl, key, NULL);
				b = ref_insert(&_ref, key);
				if (a != b) {
					printf("bad insert\n");
				}
			}
			break;
		case 1: /* delete multiple */
#if 0
			run = robmod(rand(), _ref.num);
			while (run--) {
				key = rand() % MAX_KEY;
				a = avl_delete(&_avl, key);
				b = ref_delete(&_ref, key);
				if (a != b) {
					printf("bad delete\n");
				}
			}
#endif
			break;
		case 2: /* lookup multiple */
			run = robmod(rand(), _ref.num);
			while (run--) {
				key = rand() % MAX_KEY;
				a = avl_lookup(&_avl, key, &tmp);
				b = ref_lookup(&_ref, key);
				if (a != b) {
					printf("bad lookup\n");
				}
			}
			break;
		case 3: /* consistency check */
			a = avl_check(_avl);
			if (a < 0) {
				printf("inconsistency\n");
			}
			break;
		}
		counter++;
		if (counter % 10000 == 0) {
			printf("%ju\t%ju\n", counter, _ref.num);
		}
	}
	
	avl_free(&_avl);
	ref_free(&_ref);
}

int
main()
{
	drive();
	return 0;
}


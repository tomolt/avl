#include <stdlib.h>
#include <stdio.h>

#include "avl.h"

#define MAX_KEY 10000

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

static const char *OpNames[] = {
	"INSERTION",
	"DELETION",
	"LOOKUP",
	"CONSISTENCY CHECK",
};

static void
drive(void)
{
	uintmax_t key;
	size_t run, counter, checkpoint, stride;
	void *tmp;
	int a, b;
	int valid = 1, op;

	avl_init(&_avl);
	ref_init(&_ref);
	srand(0);
	
	printf("#OPS\t#ELS\n");
	
	counter = 0;
	stride = 10000;
	checkpoint = stride;
	while (valid) {
		op = rand() % 4;
		switch (op) {
		case 0: /* insert multiple */
			run = robmod(rand(), MAX_KEY - _ref.num);
			while (run--) {
				key = rand() % MAX_KEY;
				a = avl_insert(&_avl, key, NULL);
				b = ref_insert(&_ref, key);
				if (a != b) {
					valid = 0;
				}
				counter++;
			}
			break;
		case 1: /* delete multiple */
			run = robmod(rand(), _ref.num);
			while (run--) {
				key = rand() % MAX_KEY;
				a = avl_delete(&_avl, key);
				b = ref_delete(&_ref, key);
				if (a != b) {
					valid = 0;
				}
				counter++;
			}
			break;
		case 2: /* lookup multiple */
			run = robmod(rand(), _ref.num);
			while (run--) {
				key = rand() % MAX_KEY;
				a = avl_lookup(&_avl, key, &tmp);
				b = ref_lookup(&_ref, key);
				if (a != b) {
					valid = 0;
				}
				counter++;
			}
			break;
		case 3: /* consistency check */
			a = avl_check(&_avl);
			if (a < 0) {
				valid = 0;
			}
			counter++;
			break;
		}
		if (counter >= checkpoint) {
			printf("%ju\t%ju\n", counter, _ref.num);
			stride += stride / 10;
			checkpoint += stride;
		}
		if (counter > 10000000) break;
	}

	if (valid) {
		printf("%ju\t%ju\tSUCCESS\n", counter, _ref.num);
	} else {
		printf("%ju\t%ju\t%s FAILED\n", counter, _ref.num, OpNames[op]);
		avl_print(&_avl, stdout);
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


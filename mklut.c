#include <stdio.h>
#include <assert.h>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

static void
rotate(int *a, int *b, int d)
{
	if (d) { /* CCW rotation */
		*a = *a - 1 - MAX(*b, 0);
		*b = *b - 1 + MIN(*a, 0);
	} else { /* CW rotation */
		*a = *a + 1 - MIN(*b, 0);
		*b = *b + 1 + MAX(*a, 0);
	}
}

static void
balance(int *a, int *b, int *c)
{
	int d;
	d = *a > 0;
	if (*a * *b < 0) {
		rotate(b, c, !d);
	}
	rotate(a, b, d);
}

static void
fmtlut(int lut[], const char *name)
{
	int i;
	printf("const int8_t %s[9] = { ", name);
	for (i = 0; i < 9; i++) {
		printf("%2d,", lut[i]);
	}
	printf(" };\n");
}

static void
calculate(int x, int lut_a[], int lut_c[])
{
	int a, b, c;
	int i;
	for (i = 0; i < 9; i++) {
		a = x;
		b = (i % 3) - 1;
		c = (i / 3) - 1;
		balance(&a, &b, &c);
		lut_a[i] = a;
		assert(b == -a);
		lut_c[i] = c;
	}
}

int
main()
{
	int lut_a[9], lut_c[9];

	calculate(-2, lut_a, lut_c);
	fmtlut(lut_a, "lut_cw_a ");
	fmtlut(lut_c, "lut_cw_c ");

	calculate(2, lut_a, lut_c);
	fmtlut(lut_a, "lut_ccw_a");
	fmtlut(lut_c, "lut_ccw_c");
}


CC=cc
LD=cc
CFLAGS=-g -Wall -Wextra -pedantic -std=c99
LDFLAGS=-g

.PHONY: all clean

all: avl

clean:
	rm -f *.o
	rm -f avl

avl: avl.o driver.o
	$(LD) $(LDFLAGS) $^ -o $@

avl.o: avl.c avl.h
	$(CC) $(CFLAGS) -c $< -o $@

driver.o: driver.c avl.h
	$(CC) $(CFLAGS) -c $< -o $@


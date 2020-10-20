/**********************************
 * bump_alloc.c
 * Jeremy.Singer@glasgow.ac.uk
 *
 * This is a simple bump-pointer allocator.
 * It mmaps a large buffer of SIZE bytes,
 * then allocates this space in word-sized
 * chunks to client code (in main fn).
 *
 * Initial simple memory allocator test.
 * Explore capability narrowing operations
 * and intrinsics for bound reporting.
 */

#include "include/common.h"
#include <cheriintrin.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

/* number of bytes in alloc buffer */
#define SIZE 100 * sizeof(int)

/* should program generate debugging output? 1 for Yes, 0 for No */
#define DEBUG_PRINTF 1

int count = 0;		 /* number of bytes allocated so far*/
int max = SIZE;		 /* upper limit for count */
char *buffer = NULL; /* the allocation buffer */

char *bump_alloc(int); /* the simplest malloc */
					   /* oh, and there's no free() ! */

int main()
{
	int i;

	/* request memory for our allocation buffer */
	char *res = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	if (res == MAP_FAILED)
	{
		perror("error in initial mem allocation");
		exit(-1);
	}

	/* NB mmap min bounds for capability is 1 page (4K) */
	if (DEBUG_PRINTF)
		inspect_pointer(res);

	buffer = res;

	/* now try to do some bump pointer allocations */
	for (i = 0; i < 100; i++)
	{
		int *x = (int *)bump_alloc(1 * sizeof(int));
		if (DEBUG_PRINTF)
			inspect_pointer(x);
		if (x)
		{
			*x = 42;
			if (DEBUG_PRINTF)
				printf("stored value %d\n", *x);
		}
		else
		{
			if (DEBUG_PRINTF)
				printf("bump_alloc returned null\n");
		}
	}

	return 0;
}

/*
 * allocate len bytes with bump pointer allocator
 * this is our simplistic `malloc` function
 */
char *bump_alloc(int len)
{
	char *chunk = 0;
	if (count + len <= max)
	{
		/* bump the pointer */
		chunk = buffer + count;

		/* restrict capability range before returning ptr */
		chunk = cheri_bounds_set_exact(chunk, len);

		/* update bytes allocated count */
		count += len;
	}

	return chunk;
}

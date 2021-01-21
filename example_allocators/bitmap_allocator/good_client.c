/*
 * good_client.c
 * Jeremy Singer
 *
 * Example use of bitmap allocator, showing how
 * 'good' client code should interact with the
 * allocator.
 */

#include <stdio.h>

#include "bitmap_alloc.h"
// #include "include/common.h"

/* should program generate debugging output? 1 for Yes, 0 for No */
#define DEBUG_PRINTF 1

/* size of buffer to allocate (in words) */
#define NUM_WORDS 64

int main()
{
	int i;
	int *arr;
	
	init_alloc(NUM_WORDS, sizeof(int));

	/* now try to do some bump pointer allocations */
	for (i = 0; i < NUM_WORDS; i++)
	{
		int *x = (int *)alloc_chunk();
		arr[i] = x;
		if (DEBUG_PRINTF)
		  //inspect_pointer(x);
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

	// now do some frees
	for (i = 0; i < NUM_WORDS; i++)
	{
	  free_chunk(arr[i]);
	}
	
	return 0;
}

/**********************************
 * bitmap_alloc.c
 * Jeremy.Singer@glasgow.ac.uk
 *
 * This is a simple fixed-size bitmap allocator.
 * It mmaps a large buffer of SIZE bytes,
 * then allocates this space in equally-sized
 * chunks to client code. 
 * The bitmap is used to keep track of which
 * chunks are in use (corresponding bit set to 1)
 * and which chunks are free (corresponding bit
 * set to 0).
 * 
 * This is _not_ a clever allocator, since it 
 * does a linear scan of the bitmap to find the 
 * first free chunk - expensive!
 *
 * Initial simple memory allocator test.
 * Explore capability narrowing operations
 * and intrinsics for bound reporting.
 */

// #include <cheriintrin.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "bitmap_alloc.h"

#define BITS_PER_BYTE 8
#define BYTES_PER_CHUNK 16

char *buffer = NULL; /* the allocation buffer */
char *bitmap = NULL;  /* the bitmap for the buffer */
int buffer_size = 0;  /* size of buffer (in bytes)
		      * which is the same as the 
 		      * size of the bitmap (in bits) */

void init_alloc(int size_in_bytes)
{
        int i = 0;
	/* request memory for our allocation buffer */
	char *res = mmap(NULL, size_in_bytes, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	/* request memory for our bitmap */
	// one bit per chumk - how big is each chunk?
	bitmap = mmap(NULL, size_in_bytes/(BITS_PER_BYTE * BYTES_PER_CHUNK), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	if (res == MAP_FAILED || bitmap == MAP_FAILED)
	{
		perror("error in initial mem allocation");
		exit(-1);
	}

	/* NB mmap min bounds for capability is 1 page (4K) */
	// if (DEBUG_PRINTF)
	//	inspect_pointer(res);

	buffer = res;
	buffer_size = size_in_bytes;
	// zero out bitmap - iterate?
	for (i=0; i< buffer_size/(BITS_PER_BYTE * BYTES_PER_CHUNK); i++) {
	  bitmap[i] = 0;
	}
	
	return;
}

/*
 * allocate fixed size chunk with bitmap allocator
 * this is our simplistic `malloc` function
 */
char *alloc_chunk()
{
        char updated_byte = 0;
	int chunk_index = 0;
	char *chunk = NULL;
	// iterate over all bits in bitmap, looking for a 0
	// when we find a 0, set it to 1 and
	// return the corresponding chunk
	// (setting its capability bounds)
	int i = 0;
	while (bitmap[i] == 0xff) {
	  i++;
	  if (i>=buffer_size/(BITS_PER_BYTE * BYTES_PER_CHUNK))
	    break;
	}
	// do we have a 0?
	if (i < buffer_size/(BITS_PER_BYTE * BYTES_PER_CHUNK) && bitmap[i] < 0xff) {
	  // find the lowest 0 ...
	  int j = 0;
	  // right shift until bottom bit is 0
	  for (j=0; j<8; j++) {
	    int bit = (bitmap[i] >> j) & 1;
	    if (bit == 0) {
	      break;
	    }
	  }
	  // ok - so now i is the word, j is the bit
	  // set this bit to 1 ...
	  // and work out the chunk to allocate
	  updated_byte = bitmap[i] + (1<<j);
	  bitmap[i] = updated_byte;
	  chunk_index = i*BITS_PER_BYTE + j;
	  chunk = buffer + (chunk_index * BYTES_PER_CHUNK);
	  /* restrict capability range before returning ptr */
	  /// chunk = cheri_bounds_set_exact(chunk, BYTES_PER_CHUNK);
	  // FIXME - what if this is not representable?
	}
	
	return chunk;
}

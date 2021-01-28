/**********************************
 * bitmap_alloc.c
 * Jeremy.Singer@glasgow.ac.uk
 *
 * This is a simple fixed-size bitmap allocator.
 * It mmaps a large buffer of
 * NUM_CHUNKS * CHUNK_SIZE bytes
 * then allocates this space in equally-sized
 * chunks to client code. 
 * A side bitmap is required to keep track of which
 * chunks are in use (corresponding bit set to 1)
 * and which chunks are free (corresponding bit
 * set to 0). There is one bit per allocatable chunk.
 * 
 * This is _not_ a clever allocator, since it 
 * does a linear scan of the bitmap to find the 
 * first free chunk, which is expensive!
 * More efficient scans could be easily incorporated.
 *
 * This is an initial simple memory allocator test
 * for CHERI / Capable VMs.
 * We explore capability alignment, 
 * representable bounds, narrowing operations
 * and compiler intrinsic support.
 */

// #include <cheriintrin.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "bitmap_alloc.h"

#define BITS_PER_BYTE 8

char *buffer = NULL; /* allocation buffer */
char *bitmap = NULL;  /* bitmap for the buffer */

int buffer_size = 0;  /* size of buffer (in bytes) */
int bitmap_size = 0;   /* size of bitmap (in bytes) */
int bytes_per_chunk = 0; /* size of single chunk (in bytes) */

void init_alloc(int num_chunks, int chunk_size)
{
        int i = 0;

	/* do we need to increase the num_chunks
	 * so every bit in bitmap will be used?
	 */
	int adjusted_num_chunks =
	  (num_chunks%BITS_PER_BYTE==0)?
	  num_chunks:
	  (num_chunks + (BITS_PER_BYTE-(num_chunks%BITS_PER_BYTE)));
	
	/* do we need to increase the chunk_size
	 * so chunks will be CHERI aligned?
	 * (i.e. 16 bytes for a 64-bit architecture)
	 */
	int adjusted_chunk_size =
	  (chunk_size%(sizeof(void *)) == 0)?
	  chunk_size:
	  (chunk_size + (sizeof(void *))-(chunk_size%(sizeof(void *))));

	/* request memory for our allocation buffer */
	char *res = mmap(NULL, adjusted_num_chunks*adjusted_chunk_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	/* request memory for our bitmap */
	bitmap = mmap(NULL, adjusted_num_chunks/BITS_PER_BYTE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	if (res == MAP_FAILED || bitmap == MAP_FAILED)
	{
		perror("error in initial mem allocation");
		exit(-1);
	}

	/* NB mmap min bounds for capability is 1 page (4K) */

	buffer = res;
	/* check buffer is aligned */
	assert((uintptr_t)buffer % sizeof(void *) == 0);
	/* check bitmap is aligned */
	assert((uintptr_t)bitmap % sizeof(void *) == 0);
	
	bytes_per_chunk = adjusted_chunk_size;
	buffer_size = adjusted_num_chunks * adjusted_chunk_size;
	bitmap_size = adjusted_num_chunks/BITS_PER_BYTE;
	
	/* zero bitmap, since all chunks are free initially */
	for (i=0; i < bitmap_size; i++) {
	  bitmap[i] = 0;
	}

	// FIXME - should we use CHERI API to
	// set exact bounds for buffer and bitmap?
	
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
	// printf("alloc_chunk()\n");
	while (bitmap[i] == (char)0xff) {
	  i++;
	  if (i>=bitmap_size)
	    break;
	}
	// do we have a 0?
	if (i < bitmap_size && bitmap[i] != (char)0xff) {
	  // printf("looking for j bit set to 1...\n");
	  // find the lowest 0 ...
	  int j = 0;
	  // right shift until bottom bit is 0
	  for (j=0; j<8; j++) {
	    int bit = (bitmap[i] >> j) & 1;
	    if (bit == 0) {
	      break;
	    }
	  }
	  // printf("j is %d\n", j);
	  // ok - so now i is the word, j is the bit
	  // set this bit to 1 ...
	  // and work out the chunk to allocate
	  updated_byte = bitmap[i] + (char)(1<<j);
	  bitmap[i] = updated_byte;
	  // printf("new bitmap entry %d at %d", bitmap[i], i);
	  chunk_index = i*BITS_PER_BYTE + j;
	  chunk = buffer + (chunk_index * bytes_per_chunk);
	  /* restrict capability range before returning ptr */
	  /// chunk = cheri_bounds_set_exact(chunk, BYTES_PER_CHUNK);
	  // FIXME - what if this is not representable?
	}
	//else {
 	//   printf("no room to alloc: i is %d\n bitmap_size is %d, bitmap[i] & 0xff is %d\n", i, bitmap_size, bitmap[i]&0xff);
	//}

	// CHERI set bounds for chunk
	
	return chunk;
}

void free_chunk(char *chunk) {
  // work out chunk index in buffer
  int chunk_index = (chunk - buffer) / bytes_per_chunk;
  // work out equivalent bitmap index
  int bitmap_index = chunk_index / BITS_PER_BYTE;
  int bitmap_offset = chunk_index % BITS_PER_BYTE;
  // set this bitmap value to 0
  char updated_byte = bitmap[bitmap_index] & (char)(~(1<<bitmap_offset));
  bitmap[bitmap_index] = updated_byte;
  return;
}


int num_used_chunks() {
  int i = 0;
  int used_chunks = 0;
  
  while (i<bitmap_size) {
    char x = bitmap[i];
    if (x != 0) {
      /* some used chunks here */
      char j;
      for (j=1; j<=x; j=j<<1) {
	if (x&j) {
	  used_chunks++;
	}
      }
    }
    i++;
  }
  return used_chunks;
}

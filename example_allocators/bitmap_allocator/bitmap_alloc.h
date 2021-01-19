/**********************************
 * bitmap_alloc.h
 * Jeremy.Singer@glasgow.ac.uk
 *
 * This is a simple bitmap allocator,
 * to allocate fixed-size chunks of data.
 *
 * It mmaps a large buffer of buffer_size bytes,
 * then allocates this space in fixed-sized
 * chunks of BYTES_IN_CHUNK bytes to client code.
 *
 * Initial simple memory allocator test.
 * Explore capability narrowing operations
 * and intrinsics for bound reporting.
 */

void init_alloc(int num_chunks, int chunk_size);

char *alloc_chunk(); /* simplest malloc */
                   /* free_chunk()  to follow ... */

int num_used_chunks();  /* count of used memory */

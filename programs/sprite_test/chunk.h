#include <stdint.h>

#ifndef _CHUNK_H_
#define _CHUNK_H_

struct chunk_s;
typedef struct chunk_s chunk_t;

int chunk_read_u8(chunk_t *chunk, uint8_t *dest);
int chunk_read_u16(chunk_t *chunk, uint16_t *dest);

int chunk_rewind(chunk_t *chunk);
int chunk_skip_bytes(chunk_t *chunk, size_t count);
int chunk_read_bytes(chunk_t *chunk, uint8_t *dest, size_t count);

void chunk_free(chunk_t *chunk);
chunk_t *chunk_load(const char *filename, int chunk_index);
int chunk_save(const char *filename, chunk_t *chunk);

#endif

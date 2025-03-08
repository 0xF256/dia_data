#include <stdint.h>
#include "chunk.h"
#include "texture.h"

#ifndef _SPRITE_H_
#define _SPRITE_H_

struct sprite_s;
typedef struct sprite_s sprite_t;

typedef struct
{
    int w, h;
} dim_t;

typedef struct
{
    int tex_index;
    int x, y;
    int transform;
} tile_pos_t;

typedef struct
{
    int count;
    int offset;
} pos_info_t;

size_t sprite_get_texture_count(sprite_t *sprite);
size_t sprite_get_palette_count(sprite_t *sprite);
dim_t *sprite_get_dimensions(sprite_t *sprite);
tile_pos_t *sprite_get_tile_pos(sprite_t *sprite, size_t *size);
pos_info_t *sprite_get_tile_pos_info(sprite_t *sprite, size_t *size);
pos_info_t *sprite_get_animation_info(sprite_t *sprite, size_t *size);
texture_t *sprite_get_texture(sprite_t *sprite, int texture_index, int palette_index);

void sprite_free(sprite_t *sprite);
sprite_t *sprite_load(chunk_t *sprite_chunk, int scale);

#endif
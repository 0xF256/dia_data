#include <stdint.h>
#include <stdlib.h>
#include "chunk.h"
#include "palette.h"
#include "texture.h"

#define COUNT_OF(X) (sizeof(X)/sizeof(X[0]))

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

typedef struct
{
    int         scale;

    size_t      total_textures;
    dim_t*      dimensions;
    uint8_t**   texture_data;
    uint16_t*   texture_data_len;
    uint16_t    texture_decode_type;

    size_t      total_tile_pos;
    tile_pos_t* tile_pos;

    size_t      total_tile_pos_info;
    pos_info_t* tile_pos_info;

    size_t      total_animation_info;
    pos_info_t* animation_info;

    size_t      total_palettes;
    palettes_t* palettes;
} sprite_t;

static int get_pos_info(chunk_t *chunk, int count, pos_info_t *dest)
{
    uint16_t t[2];
    for(int i = 0; i < count; i++)
    {
        if(chunk_read_bytes(chunk, (uint8_t*)t, 4)) goto fail;

        dest[i].count = t[0] & 0xFF;
        dest[i].offset = t[1];
    }
    return 0;

fail:
    return 1;
}

size_t sprite_get_texture_count(sprite_t *sprite)
{
    if(!sprite) return 0;

    return sprite->total_textures;
}

size_t sprite_get_palette_count(sprite_t *sprite)
{
    if(!sprite) return 0;

    return sprite->total_palettes;
}

dim_t *sprite_get_dimensions(sprite_t *sprite)
{
    if(!sprite) return NULL;

    return sprite->dimensions;
}

tile_pos_t *sprite_get_tile_pos(sprite_t *sprite, size_t *size)
{
    if(!sprite) return NULL;

    if(size) *size = sprite->total_tile_pos;
    return sprite->tile_pos;
}

pos_info_t *sprite_get_tile_pos_info(sprite_t *sprite, size_t *size)
{
    if(!sprite) return NULL;

    if(size) *size = sprite->total_tile_pos_info;
    return sprite->tile_pos_info;
}

pos_info_t *sprite_get_animation_info(sprite_t *sprite, size_t *size)
{
    if(!sprite) return NULL;

    if(size) *size = sprite->total_animation_info;
    return sprite->animation_info;
}

texture_t *sprite_get_texture(sprite_t *sprite, int texture_index, int palette_index)
{
    int w, h;
    palette_t *palette;

    if(!sprite) return NULL;
    if(texture_index >= sprite->total_textures || palette_index >= sprite->total_palettes) return NULL;

    w = sprite->dimensions[texture_index].w;
    h = sprite->dimensions[texture_index].h;
    palette = palette_get(sprite->palettes, palette_index);

    return texture_load(sprite->texture_data[texture_index], sprite->texture_data_len[texture_index], sprite->texture_decode_type, palette, w, h, sprite->scale);
}

void sprite_free(sprite_t *sprite)
{
    if(!sprite) return;

    for(int i = 0; i < sprite->total_textures; i++)
    {
        uint8_t *tmp = sprite->texture_data[i];
        if(tmp) free(tmp);
    }

    free(sprite->dimensions);
    free(sprite->texture_data);
    free(sprite->texture_data_len);

    if(sprite->total_tile_pos) free(sprite->tile_pos);
    if(sprite->total_tile_pos_info) free(sprite->tile_pos_info);
    if(sprite->total_animation_info) free(sprite->animation_info);

    palettes_free(sprite->palettes);
    return;
}

sprite_t *sprite_load(chunk_t *sprite_chunk, int scale)
{
    sprite_t *res;

    uint16_t texture_decode_type;

    uint16_t total_textures;
    uint16_t total_tile_pos;
    uint16_t total_tile_pos_info;
    uint16_t total_animation_info;

    uint16_t palette_type;
    uint8_t  total_palette;
    uint8_t  total_palette_pixel;

    if(!sprite_chunk) return NULL;
    if(scale <= 1) scale = 1;

    res = (sprite_t*)calloc(1, sizeof(sprite_t));
    if(!res) return NULL;

    // Check magic header
    const static uint8_t magic[] = { 0xdf, 0x03, 0x01, 0x01, 0x01, 0x01 };
    {
        uint8_t magic_tmp[COUNT_OF(magic)];
        if(chunk_read_bytes(sprite_chunk, magic_tmp, COUNT_OF(magic))) goto fail1;
        for(int i = 0; i < COUNT_OF(magic); i++)
        {
            if(magic_tmp[i] != magic[i]) goto fail1;
        }
    }

    // Store scale will use
    res->scale = scale;

    // Get how many textures in chunk
    if(chunk_read_u16(sprite_chunk, &total_textures)) goto fail1;
    if(!total_textures) goto fail1;
    res->total_textures = total_textures;

    // Get all dimensions (width, height)
    res->dimensions = (dim_t*)calloc(total_textures, sizeof(dim_t));
    if(!(res->dimensions)) goto fail1;
    for(int i = 0; i < total_textures; i++)
    {
        uint16_t dim;
        if(chunk_read_u16(sprite_chunk, &dim)) goto fail2;

        res->dimensions[i].w = (dim & 0xFF);
        res->dimensions[i].h = (dim >> 8);
    }

    // Get tile pos count
    if(chunk_read_u16(sprite_chunk, &total_tile_pos)) goto fail2;
    if((res->total_tile_pos = total_tile_pos))  // Attention here
    {
        // If have tile pos, store it
        res->tile_pos = (tile_pos_t*)calloc(total_tile_pos, sizeof(tile_pos_t));
        if(!(res->tile_pos)) goto fail2;

        uint8_t buf[4];
        for(int i = 0; i < total_tile_pos; i++)
        {
            if(chunk_read_bytes(sprite_chunk, buf, 4)) goto fail3;

            res->tile_pos[i].tex_index = buf[0];
            res->tile_pos[i].x = (int8_t)buf[1] * scale;
            res->tile_pos[i].y = (int8_t)buf[2] * scale;
            res->tile_pos[i].transform = buf[3];
        }
    }

    // Get tile pos info count
    if(chunk_read_u16(sprite_chunk, &total_tile_pos_info)) goto fail3;
    if((res->total_tile_pos_info = total_tile_pos_info))
    {
        res->tile_pos_info = (pos_info_t*)calloc(total_tile_pos_info, sizeof(pos_info_t));
        if(!(res->tile_pos_info)) goto fail3;

        if(get_pos_info(sprite_chunk, total_tile_pos_info, res->tile_pos_info)) goto fail4;
    }
    // Unknown data (part of tile pos info, ignore)
    if(chunk_skip_bytes(sprite_chunk, total_tile_pos_info * 4)) goto fail4;

    // Unknown data (ignore)
    {
        uint16_t tmp;
        if(chunk_read_u16(sprite_chunk, &tmp)) goto fail4;
        if(tmp)
        {
            if(chunk_skip_bytes(sprite_chunk, tmp * 5)) goto fail4;
        }
    }

    // Animation data
    if(chunk_read_u16(sprite_chunk, &total_animation_info)) goto fail4;
    if((res->total_animation_info = total_animation_info))
    {
        res->animation_info = (pos_info_t*)calloc(total_animation_info, sizeof(pos_info_t));
        if(!(res->animation_info)) goto fail4;

        if(get_pos_info(sprite_chunk, total_animation_info, res->animation_info)) goto fail5;
    }

    // Palette data
    if(chunk_read_u16(sprite_chunk, &palette_type)) goto fail5;
    if(chunk_read_u8(sprite_chunk, &total_palette)) goto fail5;
    if(chunk_read_u8(sprite_chunk, &total_palette_pixel)) goto fail5;
    res->total_palettes = total_palette;
    {
        int palette_data_len = total_palette * total_palette_pixel * (palette_type == 0x8888 ? 4 : 2);
        uint8_t *palette_data = (uint8_t*)malloc(palette_data_len);
        if(!palette_data) goto fail5;
        if(chunk_read_bytes(sprite_chunk, palette_data, palette_data_len))
        {
            free(palette_data);
            goto fail5;
        }
        res->palettes = palette_load(palette_data, palette_type, total_palette, total_palette_pixel);
        free(palette_data);
        if(!(res->palettes)) goto fail5;
    }

    // Texture data
    if(chunk_read_u16(sprite_chunk, &texture_decode_type)) goto fail6;
    res->texture_decode_type = texture_decode_type;
    res->texture_data = (uint8_t**)calloc(total_textures, sizeof(uint8_t*));
    res->texture_data_len = (uint16_t*)calloc(total_textures, sizeof(uint16_t));
    if(!(res->texture_data) || !(res->texture_data_len)) goto fail7;
    for(int i = 0; i < total_textures; i++)
    {
        uint16_t data_len;
        if(chunk_read_u16(sprite_chunk, &data_len)) goto fail8;
        res->texture_data_len[i] = data_len;
        uint8_t *data_tmp = res->texture_data[i] = (uint8_t*)malloc(data_len);
        if(!data_tmp) goto fail8;
        if(chunk_read_bytes(sprite_chunk, data_tmp, data_len)) goto fail8;
    }
    return res;

fail8:
    for(int i = 0; i < total_textures; i++)
    {
        uint8_t *data_tmp = res->texture_data[i];
        if(data_tmp) free(data_tmp);
    }
fail7:
    if(res->texture_data) free(res->texture_data);
    if(res->texture_data_len) free(res->texture_data_len);
fail6:
    palettes_free(res->palettes);
fail5:
    if(res->total_animation_info) free(res->animation_info);
fail4:
    if(res->total_tile_pos_info) free(res->tile_pos_info);
fail3:
    if(res->total_tile_pos) free(res->tile_pos);
fail2:
    free(res->dimensions);
fail1:
    free(res);
    return NULL;
}
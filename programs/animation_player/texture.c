#include <stdint.h>
#include <stdlib.h>
#include "palette.h"

typedef struct
{
    int w, h;
    uint32_t *pixels;
} texture_t;

// To forbid memory overflow
#define PUT(x) safe_put(pixels_tmp, x, cur_pos++, w * h)
static inline void safe_put(uint32_t *arr, uint32_t val, size_t index, size_t size)
{
    // Overflow!
    if(index >= size) return;

    arr[index] = val;
    return;
}

int texture_get_width(const texture_t *tex)
{
    if(!tex) return -1;
    return tex->w;
}

int texture_get_height(const texture_t *tex)
{
    if(!tex) return -1;
    return tex->h;
}

const uint32_t *texture_get_pixels(const texture_t *tex)
{
    if(!tex) return NULL;
    return tex->pixels;
}

void texture_free(texture_t *tex)
{
    if(!tex) return;

    free(tex->pixels);
    free(tex);
    return;
}

texture_t *texture_load(const uint8_t *decode_data, const uint16_t decode_len, const uint16_t decode_type, const palette_t *palette, int w, int h, int scale)
{
    texture_t *res;

    uint32_t *pixels;       // For scale to use
    uint32_t *pixels_tmp;   // To store unscaled pixels

    // Invalid parameter
    if(!decode_data || !palette || w <= 0 || h <= 0) return NULL;
    if(decode_type != 0x27F1 && decode_type != 0x1600 &&
       decode_type != 0x0400 && decode_type != 0x0200 &&
       decode_type != 0x5602 && decode_type != 0x56F2) return NULL;

    // Init
    scale = (scale > 1) ? scale : 0;
    if(!(res = (texture_t*)malloc(sizeof(texture_t)))) return NULL;
    pixels_tmp = (uint32_t*)calloc(w * h, sizeof(uint32_t));
    if(!pixels_tmp) goto fail1;

    size_t cur_pos = 0;
    switch(decode_type)
    {
        // F127
        case 0x27F1:
        {
            for(int i = 0; i < decode_len; i++)
            {
                uint32_t color_tmp;
                int index_tmp = decode_data[i];
                if(index_tmp <= 127)
                {
                    PUT(palette_get_color(palette, index_tmp));
                    continue;
                }

                index_tmp -= 128;
                color_tmp = palette_get_color(palette, decode_data[++i]);
                while(index_tmp-- > 0)
                {
                    PUT(color_tmp);
                }
            }
            break;
        }
        // 0016
        case 0x1600:
        {
            for(int i = 0; i < decode_len; i++)
            {
                PUT(palette_get_color(palette, (decode_data[i] >> 4) & 0xF));
                PUT(palette_get_color(palette, decode_data[i] & 0xF));
            }
            break;
        }
        // 0004
        case 0x0400:
        {
            for(int i = 0; i < decode_len; i++)
            {
                PUT(palette_get_color(palette, (decode_data[i] >> 6) & 0x3));
                PUT(palette_get_color(palette, (decode_data[i] >> 4) & 0x3));
                PUT(palette_get_color(palette, (decode_data[i] >> 2) & 0x3));
                PUT(palette_get_color(palette, decode_data[i] & 0x3));
            }
            break;
        }
        // 0002
        case 0x0200:
        {
            for(int i = 0; i < decode_len; i++)
            {
                PUT(palette_get_color(palette, (decode_data[i] >> 7) & 0x1));
                PUT(palette_get_color(palette, (decode_data[i] >> 6) & 0x1));
                PUT(palette_get_color(palette, (decode_data[i] >> 5) & 0x1));
                PUT(palette_get_color(palette, (decode_data[i] >> 4) & 0x1));
                PUT(palette_get_color(palette, (decode_data[i] >> 3) & 0x1));
                PUT(palette_get_color(palette, (decode_data[i] >> 2) & 0x1));
                PUT(palette_get_color(palette, (decode_data[i] >> 1) & 0x1));
                PUT(palette_get_color(palette, decode_data[i] & 0x1));
            }
            break;
        }
        // 0256
        case 0x5602:
        {
            for(int i = 0; i < decode_len; i++)
            {
                uint32_t color_tmp = palette_get_color(palette, decode_data[i]);
                PUT(color_tmp);
            }
            break;
        }
        // F256
        case 0x56F2:
        {
            for(int i = 0; i < decode_len; i++)
            {
                int index_tmp = decode_data[i];
                if(index_tmp <= 127)
                {
                    uint32_t color_tmp = palette_get_color(palette, decode_data[++i]);
                    while(index_tmp-- > 0)
                    {
                        PUT(color_tmp);
                    }
                    continue;
                }

                index_tmp -= 128;
                while(index_tmp-- > 0)
                {
                    PUT(palette_get_color(palette, decode_data[++i]));
                }
            }
            break;
        }
    }

    if(scale)
    {
        w *= scale;
        h *= scale;

        pixels = (uint32_t*)calloc(w * h, sizeof(uint32_t));
        if(!pixels) goto fail2;

        cur_pos = 0;
        for(int y = 0; y < h; y++)
        {
            for(int x = 0; x < w; x++)
            {
                pixels[cur_pos++] = pixels_tmp[x / scale + ((y / scale) * (w / scale))];
            }
        }
        free(pixels_tmp);
        res->pixels = pixels;
    }
    else
    {
        res->pixels = pixels_tmp;
    }
    res->w = w;
    res->h = h;
    return res;

fail2:
    free(pixels_tmp);
fail1:
    free(res);
    return NULL;
}
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define USE_PIXELFORMAT_RGBA8888

// Origin pixel format is bgra8888
// we need to invert RB channel
#ifdef USE_PIXELFORMAT_RGBA8888
  #define INVERT_RB
#endif

typedef struct
{
    int total_pixels;
    uint32_t *pixel_data;
} palette_t;

typedef struct
{
    int total_palettes;
    palette_t *palettes;
} palettes_t;

#ifdef INVERT_RB
static inline void swap(uint8_t *a, uint8_t *b)
{
    uint8_t tmp = *a;
    *a = *b;
    *b = tmp;
    return;
}
#endif

static inline void bytes_copy(const uint8_t *src, size_t offset, void *dest, size_t size)
{
    uint8_t *dst = (uint8_t*)dest;
    for(int i = 0; i < size; i++)
    {
        dst[i] = src[i + offset];
    }
    return;
}

int palette_get_count(const palettes_t *palettes)
{
    if(!palettes) return -1;

    return palettes->total_palettes;
}

palette_t *palette_get(palettes_t *palettes, size_t index)
{
    int total_palettes;
    if(!palettes) return NULL;

    total_palettes = palettes->total_palettes;
    if(index >= total_palettes) return NULL;

    return &(palettes->palettes[index]);
}

uint32_t palette_get_color(const palette_t *palette, size_t color_index)
{
    if(!palette || color_index >= palette->total_pixels)
    {
        return 0;
    }

    return palette->pixel_data[color_index];
}

void palettes_free(palettes_t *palettes)
{
    if(!palettes) return;

    int total_palette = palette_get_count(palettes);
    if(total_palette > 0)
    {
        palette_t *palettes_tmp = palettes->palettes;
        for(int i = 0; i < total_palette; i++)
        {
            if(palettes_tmp[i].pixel_data)
            {
                free(palettes_tmp[i].pixel_data);
            }
        }
        free(palettes_tmp);
    }
    free(palettes);
    return;
}

palettes_t *palette_load(const uint8_t *data, uint16_t pixel_format, uint8_t total_palette, uint8_t total_pixels)
{
    if(!total_palette || !total_pixels) return NULL;
    if(!(pixel_format == 0x8888 || pixel_format == 0x4444 || pixel_format == 0x5515 || pixel_format == 0x6505)) return NULL;
    palettes_t *res = (palettes_t*)malloc(sizeof(palettes_t));
    palette_t *palettes_tmp = (palette_t*)calloc(total_palette, sizeof(palette_t));
    if(!res || !palettes_tmp)
    {
        if(res) free(res);
        if(palettes_tmp) free(palettes_tmp);
        return NULL;
    }

    int cur_pos = 0;
    for(int i = 0; i < total_palette; i++)
    {
        uint32_t *t = (uint32_t*)calloc(total_pixels, sizeof(uint32_t));
        if(!t)
        {
            cur_pos += pixel_format == 0x8888 ? 4 * total_pixels : 2 * total_pixels;
            palettes_tmp[i].total_pixels = 0;
            palettes_tmp[i].pixel_data = NULL;
            continue;
        }

        switch(pixel_format)
        {
            // A8R8G8B8
            case 0x8888:
            {
                for(int j = 0; j < total_pixels; j++)
                {
                    uint32_t c;
                    bytes_copy((const uint8_t*)data, cur_pos, (void*)&c, sizeof(c));
                    cur_pos += 4;
                    if((c & 0xFF000000) != 0xFF000000)
                    {
                        // have_alpha = true;
                    }
                    t[j] = c;
                }
                break;
            }
            // A4R4GB4
            case 0x4444:
            {
                for(int j = 0; j < total_pixels; j++)
                {
                    uint16_t c;
                    bytes_copy((const uint8_t*)data, cur_pos, (void*)&c, sizeof(c));
                    cur_pos += 2;
                    if((c & 0xF000) != 0xF000)
                    {
                        // have_alpha = true;
                    }
                    t[j] = ((c & 0xF000) << 16 |    // A
                            (c & 0xF000) << 12 |    // A
                            (c & 0x0F00) << 12 |    // R
                            (c & 0x0F00) << 8 |     // R
                            (c & 0x00F0) << 8 |     // G
                            (c & 0x00F0) << 4 |     // G
                            (c & 0x000F) << 4 |     // B
                            (c & 0x000F));          // B
                }
                break;
            }
            // A1R5G5B5
            case 0x5515:
            {
                for(int j = 0; j < total_pixels; j++)
                {
                    uint16_t c;
                    bytes_copy((const uint8_t*)data, cur_pos, (void*)&c, sizeof(c));
                    cur_pos += 2;
                    unsigned int alpha = 0xFF000000;
                    if((c & 0x8000) != 0x8000)
                    {
                        // alpha = 0;
                        // have_alpha = true;
                        t[j] = 0;
                        continue;
                    }
                    t[j] = (alpha | (c & 0x7C00) << 9 | (c & 0x3E0) << 6 | (c & 0x1F) << 3);
                }
                break;
            }
            // A0R5G6B5
            case 0x6505:
            {
                for(int j = 0; j < total_pixels; j++)
                {
                    uint16_t c;
                    bytes_copy((const uint8_t*)data, cur_pos, (void*)&c, sizeof(c));
                    cur_pos += 2;
                    unsigned int alpha = 0xFF000000;
                    if(c == 0xF81F)
                    {
                        // alpha = 0;
                        // have_alpha = true;
                        t[j] = 0;
                        continue;
                    }
                    t[j] = (alpha | (c & 0xF800) << 8 | (c & 0x7E0) << 5 | (c & 0x1F) << 3);
                }
                break;
            }
        }
#ifdef INVERT_RB
        for(int j = 0; j < total_pixels; j++)
        {
            swap((uint8_t*)&t[j], ((uint8_t*)&t[j]) + 2);
        }
#endif
        palettes_tmp[i].total_pixels = total_pixels;
        palettes_tmp[i].pixel_data = t;
    }
    res->total_palettes = total_palette;
    res->palettes = palettes_tmp;
    return res;
}

/*
 * MIT License
 * 
 * Copyright (c) 2025 SmithGoll
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "palette.h"

// #define USE_PIXELFORMAT_RGBA8888

#ifdef USE_PIXELFORMAT_RGBA8888
#define INVERT_RB
#endif

#ifdef INVERT_RB
static inline void swap_impl(uint8_t* a, uint8_t* b)
{
    uint8_t tmp = *a;
    *a = *b;
    *b = tmp;
    return;
}
#endif

static inline void bytes_copy(const uint8_t* src, size_t offset, void* dest, size_t size)
{
    uint8_t* dst = (uint8_t*)dest;
    for (int i = 0; i < size; i++) {
        dst[i] = src[i + offset];
    }
    return;
}

int palette_get_count(const palettes_t* palettes)
{
    if (!palettes)
        return -1;

    return palettes->total_palettes;
}

palette_t* palette_get(palettes_t* palettes, size_t index)
{
    int total_palettes;
    if (!palettes)
        return NULL;

    total_palettes = palettes->total_palettes;
    if (index >= total_palettes)
        return NULL;

    return &(palettes->palettes[index]);
}

uint32_t palette_get_color(const palette_t* palette, size_t color_index)
{
    if (!palette || color_index >= palette->total_pixels)
        return 0;

    return palette->pixel_data[color_index];
}

void palettes_free(palettes_t* palettes)
{
    if (!palettes)
        return;

    int total_palette = palette_get_count(palettes);
    if (total_palette > 0) {
        palette_t* palettes_tmp = palettes->palettes;
        for (int i = 0; i < total_palette; i++) {
            if (palettes_tmp[i].pixel_data) {
                free(palettes_tmp[i].pixel_data);
            }
        }
        free(palettes_tmp);
    }
    free(palettes);
    return;
}

palettes_t* palettes_load(const uint8_t* data, uint16_t pixel_format, int total_palette, int total_pixels)
{
    palettes_t* res;
    palette_t* palettes_tmp;

    int cur_pos;

    if (total_palette <= 0 || total_pixels <= 0)
        return NULL;
    if (!palette_get_format_size(pixel_format))
        return NULL;

    res = (palettes_t*)malloc(sizeof(palettes_t));
    palettes_tmp = (palette_t*)calloc(total_palette, sizeof(palette_t));
    if (!res || !palettes_tmp) {
        if (res) free(res);
        if (palettes_tmp) free(palettes_tmp);
        return NULL;
    }

    cur_pos = 0;
    for (int i = 0; i < total_palette; i++) {
        uint32_t* t = (uint32_t*)calloc(total_pixels, sizeof(uint32_t));
        if (!t) {
            cur_pos += palette_get_format_size(pixel_format) * total_pixels;
            palettes_tmp[i].total_pixels = 0;
            palettes_tmp[i].pixel_data = NULL;
            continue;
        }

        switch (pixel_format) {
        case PIXEL_FORMAT_8888: {
            for (int j = 0; j < total_pixels; j++) {
                uint32_t c;
                bytes_copy((const uint8_t*)data, cur_pos, (void*)&c,
                    sizeof(c));
                cur_pos += 4;
                if ((c & 0xFF000000) != 0xFF000000) {
                    // have_alpha = true;
                }
                t[j] = c;
            }
            break;
        }

        case PIXEL_FORMAT_4444: {
            for (int j = 0; j < total_pixels; j++) {
                uint16_t c;
                bytes_copy((const uint8_t*)data, cur_pos, (void*)&c,
                    sizeof(c));
                cur_pos += 2;
                if ((c & 0xF000) != 0xF000) {
                    // have_alpha = true;
                }
                t[j] = ((c & 0xF000) << 16 | // A
                    (c & 0xF000) << 12 | // A
                    (c & 0x0F00) << 12 | // R
                    (c & 0x0F00) << 8 | // R
                    (c & 0x00F0) << 8 | // G
                    (c & 0x00F0) << 4 | // G
                    (c & 0x000F) << 4 | // B
                    (c & 0x000F)); // B
            }
            break;
        }

        case PIXEL_FORMAT_1555: {
            for (int j = 0; j < total_pixels; j++) {
                uint16_t c;
                bytes_copy((const uint8_t*)data, cur_pos, (void*)&c,
                    sizeof(c));
                cur_pos += 2;
                unsigned int alpha = 0xFF000000;
                if ((c & 0x8000) != 0x8000) {
                    // alpha = 0;
                    // have_alpha = true;
                    t[j] = 0;
                    continue;
                }
                t[j] = (alpha | (c & 0x7C00) << 9 | (c & 0x3E0) << 6 | (c & 0x1F) << 3);
            }
            break;
        }

        case PIXEL_FORMAT_0565: {
            for (int j = 0; j < total_pixels; j++) {
                uint16_t c;
                bytes_copy((const uint8_t*)data, cur_pos, (void*)&c,
                    sizeof(c));
                cur_pos += 2;
                unsigned int alpha = 0xFF000000;
                if (c == 0xF81F) {
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
        for (int j = 0; j < total_pixels; j++) {
            swap_impl((uint8_t*)&t[j], ((uint8_t*)&t[j]) + 2);
        }
#endif
        palettes_tmp[i].total_pixels = total_pixels;
        palettes_tmp[i].pixel_data = t;
    }
    res->total_palettes = total_palette;
    res->palettes = palettes_tmp;
    return res;
}

int palette_get_format_size(uint16_t pixel_format)
{
    switch (pixel_format) {
    case PIXEL_FORMAT_8888:
        return 4;
    case PIXEL_FORMAT_4444:
    case PIXEL_FORMAT_1555:
    case PIXEL_FORMAT_0565:
        return 2;
    }
    return 0;
}

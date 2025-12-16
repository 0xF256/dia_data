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

#include "texture.h"

#ifndef FREE_PIXEL_DATA
    #define FREE_PIXEL_DATA 1
#endif

// Forbid memory overflow
#define SAFE_PUT(x) safe_put(pixels, x, cur_pos++, w * h)
static inline void safe_put(uint32_t* arr, uint32_t val, size_t index, size_t size)
{
    // Overflow!
    if (index >= size)
        return;

    arr[index] = val;
    return;
}

void* texture_load(const uint8_t* data, int data_len, uint16_t encode_format, const palette_t* palette, int w, int h, void* user_data)
{
    void* res;

    size_t cur_pos;
    uint32_t* pixels;

    // param check
    if (!data || !palette || w <= 0 || h <= 0)
        return NULL;

    pixels = (uint32_t*)calloc(w * h, sizeof(uint32_t));
    if (!pixels)
        goto fail;

    cur_pos = 0;
    switch (encode_format) {
    // 0002
    case ENCODE_FORMAT_I2: {
        for (int i = 0; i < data_len; i++) {
            SAFE_PUT(palette_get_color(palette, (data[i] >> 7) & 0x1));
            SAFE_PUT(palette_get_color(palette, (data[i] >> 6) & 0x1));
            SAFE_PUT(palette_get_color(palette, (data[i] >> 5) & 0x1));
            SAFE_PUT(palette_get_color(palette, (data[i] >> 4) & 0x1));
            SAFE_PUT(palette_get_color(palette, (data[i] >> 3) & 0x1));
            SAFE_PUT(palette_get_color(palette, (data[i] >> 2) & 0x1));
            SAFE_PUT(palette_get_color(palette, (data[i] >> 1) & 0x1));
            SAFE_PUT(palette_get_color(palette, data[i] & 0x1));
        }
        break;
    }
    // 0004
    case ENCODE_FORMAT_I4: {
        for (int i = 0; i < data_len; i++) {
            SAFE_PUT(palette_get_color(palette, (data[i] >> 6) & 0x3));
            SAFE_PUT(palette_get_color(palette, (data[i] >> 4) & 0x3));
            SAFE_PUT(palette_get_color(palette, (data[i] >> 2) & 0x3));
            SAFE_PUT(palette_get_color(palette, data[i] & 0x3));
        }
        break;
    }
    // 0016
    case ENCODE_FORMAT_I16: {
        for (int i = 0; i < data_len; i++) {
            SAFE_PUT(palette_get_color(palette, (data[i] >> 4) & 0xF));
            SAFE_PUT(palette_get_color(palette, data[i] & 0xF));
        }
        break;
    }
    // 0256
    case ENCODE_FORMAT_I256: {
        for (int i = 0; i < data_len; i++) {
            uint32_t color_tmp = palette_get_color(palette, data[i]);
            SAFE_PUT(color_tmp);
        }
        break;
    }
    // F127
    case ENCODE_FORMAT_I127RLE: {
        for (int i = 0; i < data_len; i++) {
            uint32_t color_tmp;
            int index_tmp = data[i];
            if (index_tmp <= 127) {
                SAFE_PUT(palette_get_color(palette, index_tmp));
                continue;
            }

            index_tmp -= 128;
            color_tmp = palette_get_color(palette, data[++i]);
            while (index_tmp-- > 0) {
                SAFE_PUT(color_tmp);
            }
        }
        break;
    }
    // F256
    case ENCODE_FORMAT_I256RLE: {
        for (int i = 0; i < data_len; i++) {
            int index_tmp = data[i];
            if (index_tmp <= 127) {
                uint32_t color_tmp = palette_get_color(palette, data[++i]);
                while (index_tmp-- > 0) {
                    SAFE_PUT(color_tmp);
                }
                continue;
            }

            index_tmp -= 128;
            while (index_tmp-- > 0) {
                SAFE_PUT(palette_get_color(palette, data[++i]));
            }
        }
        break;
    }
    // Unsupported format
    default:
        goto fail;
    }

    res = module_new(pixels, w, h, user_data);
#ifdef FREE_PIXEL_DATA
    free(pixels);
#endif
    return res;

fail:
    if (pixels)
        free(pixels);

    return NULL;
}

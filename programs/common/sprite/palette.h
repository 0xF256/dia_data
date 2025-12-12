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

#ifndef _PALETTE_H_
#define _PALETTE_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// defines
#define PIXEL_FORMAT_8888 0x8888
#define PIXEL_FORMAT_4444 0x4444
#define PIXEL_FORMAT_1555 0x5515
#define PIXEL_FORMAT_0565 0x6505

// structs
typedef struct palette_s
{
    int total_pixels;
    uint32_t *pixel_data;
} palette_t;

typedef struct palettes_s
{
    int total_palettes;
    palette_t *palettes;
} palettes_t;

// public functions
int         palette_get_count   (const palettes_t *palettes);

palette_t*  palette_get         (palettes_t *palettes,
                                     size_t index);

uint32_t    palette_get_color   (const palette_t *palette,
                                          size_t color_index);

void        palettes_free       (palettes_t *palettes);

palettes_t* palettes_load       (const uint8_t *data,
                                      uint16_t pixel_format,
                                           int total_palette,
                                           int total_pixels);

int palette_get_format_size(uint16_t pixel_format);

#ifdef __cplusplus
}
#endif

#endif
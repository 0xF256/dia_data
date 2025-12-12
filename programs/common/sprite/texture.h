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

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <stdint.h>
#include <stdlib.h>
#include "palette.h"
#include "engine_tex_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

// defines
#define ENCODE_FORMAT_I2        0x0200
#define ENCODE_FORMAT_I4        0x0400
#define ENCODE_FORMAT_I16       0x1600
#define ENCODE_FORMAT_I256      0x5602
#define ENCODE_FORMAT_I127RLE   0x27F1
#define ENCODE_FORMAT_I256RLE   0x56F2

// public functions
void* texture_load(const uint8_t *data,
                    int data_len,
                    uint16_t encode_format,
                    const palette_t *palette,
                    int w, int h);

#ifdef __cplusplus
}
#endif

#endif
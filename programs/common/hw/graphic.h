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

#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

// structs
struct graphic_s;
typedef struct graphic_s graphic_t;

struct tex_s;
typedef struct tex_s tex_t;

// public functions
int graphic_init(const char *window_name, int width, int height);

int graphic_get_width();

int graphic_get_height();

void graphic_show_window();

void graphic_draw_region(tex_t *texture, int x, int y, int transform);

void graphic_clear();

void graphic_present();

void graphic_quit();

int graphic_get_texture_width(tex_t *tex);

int graphic_get_texture_height(tex_t *tex);

tex_t *graphic_create_texture(const void *pixels, int w, int h, int pitch);

void graphic_destroy_texture(tex_t *tex);

#ifdef __cplusplus
}
#endif

#endif

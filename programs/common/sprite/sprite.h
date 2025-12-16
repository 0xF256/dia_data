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

#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <stdint.h>
#include <stdlib.h>
#include "file_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

// defines
#define FLIP_X  (0x1)
#define FLIP_Y  (0x2)
#define FLIP_XY (FLIP_X|FLIP_Y)

// structs
typedef struct dim_s
{
    int w, h;
} dim_t;

typedef struct fmodule_s
{
    int x, y;
    int module_index;
    int flip;
} fmodule_t;

typedef struct frame_s
{
    int count;
    int offset;
} frame_t;

typedef struct frame_rect_s
{
    int x, y;
    int w, h;
} frame_rect_t;

typedef struct aframe_s
{
    int x, y;
    int frame_index;
    int flip;
    int time;
} aframe_t;

typedef struct anim_s
{
    int count;
    int offset;
} anim_t;

typedef struct sprite_s
{
    int module_count;
    dim_t *module_dims;
    void **modules;

    int palette_count;
    void *palettes;

    int fmodule_count;
    fmodule_t *fmodules;

    int frame_count;
    frame_t *frames;
    frame_rect_t *frame_rects;

    int aframe_count;
    aframe_t *aframes;

    int anim_count;
    anim_t *anims;

    // for palette switch
    int cur_palette;
    void *private_data;

    // for texture create
    void *user_data;
} sprite_t;

// public functions

// experimental abs position functions, do not use
int sprite_get_abs_frame_vertex (sprite_t *spr, int frame_index, int flip, int *x, int *y);
void sprite_draw_aframe_abs     (sprite_t *spr, int af_index,      int x, int y, int flip);
void sprite_draw_frame_abs      (sprite_t *spr, int frame_index,   int x, int y, int flip);

void sprite_draw_aframe         (sprite_t *spr, int af_index,      int x, int y, int flip);
void sprite_draw_frame          (sprite_t *spr, int frame_index,   int x, int y, int flip);
void sprite_draw_frame_module   (sprite_t *spr, int fm_index,      int x, int y, int flip);
void sprite_draw_module         (sprite_t *spr, int module_index,  int x, int y, int flip);

int  sprite_change_palette   (sprite_t *spr, int pal_index);
void sprite_free_module_cache(sprite_t *spr, int pal_index);

void        sprite_free   (sprite_t *spr);
sprite_t*   sprite_load   (file_handle_t *handle, void *user_data);

#ifdef __cplusplus
}
#endif

#endif
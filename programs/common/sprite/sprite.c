/* MIT License
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

#include "sprite.h"

#include <stdio.h> /* fprintf */
#include <string.h> /* memset */

#include "engine_tex_impl.h"
#include "palette.h"
#include "texture.h"

#define FAIL_STRING() ("Failed at %s in %s[%d]\n")
#define FAIL_STRING_EX(fmt) ("Failed at %s(returned " fmt ") in %s[%d]\n")
#define FAIL_REPORT() \
    fprintf(stderr, FAIL_STRING(), __func__, __FILE__, __LINE__)
#define FAIL_REPORT_EX(fmt, val) \
    fprintf(stderr, FAIL_STRING_EX(fmt), __func__, (val), __FILE__, __LINE__)

#define FAIL()         \
    do {               \
        FAIL_REPORT(); \
        goto fail;     \
    } while (0)

// private struct
typedef struct info_s {
    int data_len;
    int data_offset;
} info_t;

typedef struct priv_data_s {
    info_t* infos;
    uint8_t* data;
    uint16_t encode_format;
} priv_data_t;

// global variables

// private functions
static void* ptr_offs(void* p, int offs)
{
    uint8_t* t = (uint8_t*)p;
    return (void*)(t + offs);
}

// public functions
void sprite_draw_aframe(sprite_t* spr, int af_index, int x, int y, int flip)
{
    int frame_index;
    int off_x, off_y;

    if (!spr || af_index < 0)
        return;
    if (af_index >= spr->aframe_count)
        return;

    off_x = spr->aframes[af_index].x;
    off_y = spr->aframes[af_index].y;
    flip ^= spr->aframes[af_index].flip;
    frame_index = spr->aframes[af_index].frame_index;

    sprite_draw_frame(spr, frame_index, x + off_x, y + off_y, flip);
    return;
}

void sprite_draw_frame(sprite_t* spr, int frame_index, int x, int y, int flip)
{
    int count;
    int offset;

    if (!spr || frame_index < 0)
        return;
    if (frame_index >= spr->frame_count)
        return;

    count = spr->frames[frame_index].count;
    offset = spr->frames[frame_index].offset;
    for (int i = 0; i < count; i++) {
        sprite_draw_frame_module(spr, i + offset, x, y, flip);
    }
    return;
}

void sprite_draw_frame_module(sprite_t* spr, int fm_index, int x, int y, int flip)
{
    int module_index;
    int module_w, module_h;
    int off_x, off_y;

    if (!spr || fm_index < 0)
        return;
    if (fm_index >= spr->fmodule_count)
        return;

    off_x = spr->fmodules[fm_index].x;
    off_y = spr->fmodules[fm_index].y;
    flip ^= spr->fmodules[fm_index].flip;
    module_index = spr->fmodules[fm_index].module_index;
    module_w = spr->module_dims[module_index].w;
    module_h = spr->module_dims[module_index].h;

    sprite_draw_module(spr, module_index, x + off_x, y + off_y, flip);
    return;
}

void sprite_draw_module(sprite_t* spr, int module_index, int x, int y, int flip)
{
    int cur_pal;
    void* module;

    if (!spr || module_index < 0)
        return;
    if (module_index >= spr->module_count)
        return;

    cur_pal = spr->cur_palette;
    module_index += cur_pal * spr->module_count;
    module = spr->modules[module_index];
    if (!module) {
        if (sprite_change_palette(spr, cur_pal))
            return;
        module = spr->modules[module_index];
    }
    module_paint(module, x, y, flip);
    return;
}

int sprite_change_palette(sprite_t* spr, int pal_index)
{
    int offset;
    int module_count;
    void* temp;
    dim_t* dims;
    void** modules;
    palette_t* pal;

    int data_len;
    int data_offset;
    int tex_w, tex_h;
    uint16_t encode_format;
    uint8_t* tex_data;
    info_t* infos;
    priv_data_t* private_data;

    if (!spr || pal_index < 0)
        return -1;
    if (pal_index >= spr->palette_count)
        return -2;

    dims = spr->module_dims;
    modules = spr->modules;
    module_count = spr->module_count;
    offset = pal_index * module_count;
    pal = palette_get((palettes_t*)(spr->palettes), pal_index);

    private_data = (priv_data_t*)spr->private_data;
    encode_format = private_data->encode_format;
    tex_data = private_data->data;
    infos = private_data->infos;

    spr->cur_palette = pal_index;
    for (int i = 0; i < module_count; i++) {
        int idx = i + offset;

        temp = modules[idx];
        if (temp)
            continue;

        tex_w = dims[i].w;
        tex_h = dims[i].h;
        data_len = infos[i].data_len;
        data_offset = infos[i].data_offset;
        temp = texture_load(tex_data + data_offset, data_len, encode_format,
            pal, tex_w, tex_h);
        modules[idx] = temp;
    }
    return 0;
}

void sprite_free_module_cache(sprite_t* spr, int pal_index)
{
    int offset;
    int module_count;
    void* temp;
    void** modules;

    if (!spr || pal_index < 0)
        return;
    if (pal_index >= spr->palette_count)
        return;

    modules = spr->modules;
    module_count = spr->module_count;
    offset = pal_index * module_count;
    for (int i = 0; i < module_count; i++) {
        int idx = i + offset;

        temp = modules[idx];
        if (temp) {
            module_free(temp);
            modules[idx] = NULL;
        }
    }
    return;
}

void sprite_free(sprite_t* spr)
{
    int pal_count;
    palettes_t* palettes;

    if (!spr)
        return;

    pal_count = spr->palette_count;
    palettes = (palettes_t*)(spr->palettes);
    for (int i = 0; i < pal_count; i++) {
        sprite_free_module_cache(spr, i);
    }
    palettes_free(palettes);

    free(spr);
    return;
}

sprite_t* sprite_load(file_handle_t* handle)
{
    size_t file_offset;

    int needed_size;

    void* p;
    sprite_t* res;

    // temps
    uint8_t u8t;
    uint16_t u16t;
    uint8_t* buffer; // store palettes data

    dim_t* module_dims;
    void** modules;
    fmodule_t* fmodules;
    frame_t* frames;
    frame_rect_t* frame_rects;
    aframe_t* aframes;
    anim_t* anims;
    info_t* infos;
    priv_data_t* priv_data;
    uint8_t* encode_data;

    // counter
    int module_count;
    int fmodule_count;
    int frame_count;
    int aframe_count;
    int anim_count;

    int palette_size;
    int palette_count;
    int color_count;
    int pixel_size;
    uint16_t pixel_format;
    palettes_t* palettes;

    int encode_data_off;
    uint16_t encode_format;
    int encode_data_size;

    res = NULL;
    buffer = NULL;
    needed_size = 0;

    // check magic header
    const static uint8_t magic[] = { 0xdf, 0x03, 0x01, 0x01, 0x01, 0x01 };
    {
        uint8_t magic_tmp[sizeof(magic)];
        if (file_read(handle, magic_tmp, sizeof(magic_tmp))) FAIL();
        for (int i = 0; i < sizeof(magic); i++) {
            if (magic_tmp[i] != magic[i]) FAIL();
        }
    }

    if (file_pos(handle, &file_offset)) FAIL();

    // first pass for needed memory size
    file_set_global_endian(ENDIAN_LE);

    // Module
    if (file_get_u16(handle, &u16t)) FAIL();
    module_count = u16t;
    if (file_seek(handle, module_count * 2, FSEEK_CUR)) FAIL();

    // FModule
    if (file_get_u16(handle, &u16t)) FAIL();
    fmodule_count = u16t;
    if (fmodule_count) {
        if (file_seek(handle, fmodule_count * 4, FSEEK_CUR)) FAIL();
    }

    // Frame
    if (file_get_u16(handle, &u16t)) FAIL();
    frame_count = u16t;
    if (frame_count) {
        if (file_seek(handle, frame_count * 8, FSEEK_CUR)) FAIL();
    }

    // AFrame
    if (file_get_u16(handle, &u16t)) FAIL();
    aframe_count = u16t;
    if (aframe_count) {
        if (file_seek(handle, aframe_count * 5, FSEEK_CUR)) FAIL();
    }

    // Anim
    if (file_get_u16(handle, &u16t)) FAIL();
    anim_count = u16t;
    if (anim_count) {
        if (file_seek(handle, anim_count * 4, FSEEK_CUR)) FAIL();
    }

    // Palette
    if (file_get_u16(handle, &pixel_format)) FAIL();
    pixel_size = palette_get_format_size(pixel_format);
    if (file_get_u8(handle, &u8t)) FAIL();
    palette_count = u8t;
    if (file_get_u8(handle, &u8t)) FAIL();
    color_count = u8t;
    palette_size = pixel_size * palette_count * color_count;
    if (file_seek(handle, palette_size, FSEEK_CUR)) FAIL();

    // Module image data
    encode_data_size = 0;
    if (file_get_u16(handle, &encode_format)) FAIL();
    for (int i = 0; i < module_count; i++) {
        if (file_get_u16(handle, &u16t)) FAIL();
        if (file_seek(handle, u16t, FSEEK_CUR)) FAIL();
        encode_data_size += u16t;
    }

    // calculate memory size and allocate
    p = NULL;
    fmodules = NULL;
    frames = NULL;
    frame_rects = NULL;
    aframes = NULL;
    anims = NULL;

    while (1) {
        // for sprite_t struct
        if (p)
            res = (sprite_t*)ptr_offs(p, 0);
        needed_size += sizeof(sprite_t);

        // for spr->module_dims
        if (p)
            module_dims = (dim_t*)ptr_offs(p, needed_size);
        needed_size += module_count * sizeof(dim_t);

        // for spr->modules
        if (p)
            modules = (void**)ptr_offs(p, needed_size);
        needed_size += module_count * palette_count * sizeof(void*);

        // for spr->fmodules
        if (fmodule_count) {
            if (p)
                fmodules = (fmodule_t*)ptr_offs(p, needed_size);
            needed_size += fmodule_count * sizeof(fmodule_t);
        }

        if (frame_count) {
            // for spr->frames
            if (p)
                frames = (frame_t*)ptr_offs(p, needed_size);
            needed_size += frame_count * sizeof(frame_t);

            // for spr->frame_rects
            if (p)
                frame_rects = (frame_rect_t*)ptr_offs(p, needed_size);
            needed_size += frame_count * sizeof(frame_rect_t);
        }

        // for spr->aframes
        if (aframe_count) {
            if (p)
                aframes = (aframe_t*)ptr_offs(p, needed_size);
            needed_size += aframe_count * sizeof(aframe_t);
        }

        // for spr->anims
        if (anim_count) {
            if (p)
                anims = (anim_t*)ptr_offs(p, needed_size);
            needed_size += anim_count * sizeof(anim_t);
        }

        // for spr->private_data struct
        if (p)
            priv_data = (priv_data_t*)ptr_offs(p, needed_size);
        needed_size += sizeof(priv_data_t);

        // for spr->private_data->infos
        if (p)
            infos = (info_t*)ptr_offs(p, needed_size);
        needed_size += module_count * sizeof(info_t);

        // for spr->private_data->data
        if (p)
            encode_data = (uint8_t*)ptr_offs(p, needed_size);
        needed_size += encode_data_size;

        if (p)
            break;

        p = malloc(needed_size);
        if (!p) FAIL();
        memset(p, 0, needed_size);
        needed_size = 0;
    }
#define SET(x) res->x = x
    SET(module_count);
    SET(module_dims);
    SET(modules);
    SET(fmodule_count);
    SET(fmodules);
    SET(frame_count);
    SET(frames);
    SET(frame_rects);
    SET(aframe_count);
    SET(aframes);
    SET(anim_count);
    SET(anims);
    SET(palette_count);
#undef SET
    priv_data->infos = infos;
    priv_data->data = encode_data;
    priv_data->encode_format = encode_format;
    res->private_data = (void*)priv_data;

    // second pass for parsing
    if (file_seek(handle, file_offset, FSEEK_SET)) FAIL();

    // Module
    if (file_seek(handle, 2, FSEEK_CUR)) FAIL();
    for (int i = 0; i < module_count; i++) {
        uint8_t t[2];
        if (file_read(handle, t, 2)) FAIL();

        module_dims[i].w = t[0];
        module_dims[i].h = t[1];
    }

    // FModule
    if (file_seek(handle, 2, FSEEK_CUR)) FAIL();
    for (int i = 0; i < fmodule_count; i++) {
        uint8_t t[4];
        if (file_read(handle, t, 4)) FAIL();

        fmodules[i].module_index = t[0];
        fmodules[i].x = (int8_t)t[1];
        fmodules[i].y = (int8_t)t[2];
        fmodules[i].flip = t[3];
    }

    // Frame
    if (file_seek(handle, 2, FSEEK_CUR)) FAIL();
    for (int i = 0; i < frame_count; i++) {
        uint16_t t[2];
        if (file_get_u16(handle, &t[0])) FAIL();
        if (file_get_u16(handle, &t[1])) FAIL();

        frames[i].count = t[0];
        frames[i].offset = t[1];
    }

    // Frame rect
    for (int i = 0; i < frame_count; i++) {
        uint8_t t[4];
        if (file_read(handle, t, 4)) FAIL();

        frame_rects[i].x = (int8_t)t[0];
        frame_rects[i].y = (int8_t)t[1];
        frame_rects[i].w = t[2];
        frame_rects[i].h = t[3];
    }

    // AFrame
    if (file_seek(handle, 2, FSEEK_CUR)) FAIL();
    for (int i = 0; i < aframe_count; i++) {
        uint8_t t[5];
        if (file_read(handle, t, 5)) FAIL();

        aframes[i].frame_index = t[0];
        aframes[i].time = t[1];
        aframes[i].x = (int8_t)t[2];
        aframes[i].y = (int8_t)t[3];
        aframes[i].flip = t[4];
    }

    // Anim
    if (file_seek(handle, 2, FSEEK_CUR)) FAIL();
    for (int i = 0; i < anim_count; i++) {
        uint16_t t[2];
        if (file_get_u16(handle, &t[0])) FAIL();
        if (file_get_u16(handle, &t[1])) FAIL();

        anims[i].count = t[0];
        anims[i].offset = t[1];
    }

    // Palette
    if (file_seek(handle, 4, FSEEK_CUR)) FAIL();
    buffer = (uint8_t*)malloc(palette_size);
    if (!buffer) FAIL();
    if (file_read(handle, buffer, palette_size)) FAIL();
    palettes = palettes_load(buffer, pixel_format, palette_count, color_count);
    if (!palettes) FAIL();
    res->palettes = (void*)palettes;
    free(buffer);

    // Module image data
    encode_data_off = 0;
    if (file_seek(handle, 2, FSEEK_CUR)) FAIL();
    for (int i = 0; i < module_count; i++) {
        if (file_get_u16(handle, &u16t)) FAIL();
        infos[i].data_len = u16t;
        infos[i].data_offset = encode_data_off;

        if (file_read(handle, encode_data + encode_data_off, u16t)) FAIL();
        encode_data_off += u16t;
    }

    sprite_change_palette(res, 0);
    return res;

fail:
    if (buffer)
        free(buffer);
    if (res)
        sprite_free(res);
    return NULL;
}

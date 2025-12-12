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

#include "chunk.h"
#include <stdlib.h>
#include <string.h>

// private structs
struct chunk_s {
    int count;
    int* size;
    int* offset;
    uint8_t* data;
};

typedef struct chunk_handle_s {
    size_t pos;
    size_t size;
    uint8_t* data;
} chunk_handle_t;

// private functions statement
static void* chunk_open_impl(const char* filename, const char* mode);
static void chunk_close_impl(void* handle);
static int chunk_read_impl(void* handle, void* dst, size_t count);
static int chunk_write_impl(void* handle, const void* src, size_t count);
static int chunk_seek_impl(void* handle, long offset, seek_mode_t mode);
static int chunk_pos_impl(void* handle, size_t* pos);
static void* chunk_dup_impl(void* handle);
static size_t chunk_size_impl(void* handle);

// private variables
const static file_op_t chunk_handle_op = {
    chunk_open_impl,
    chunk_close_impl,
    chunk_read_impl,
    chunk_write_impl,
    chunk_seek_impl,
    chunk_pos_impl,
    chunk_dup_impl,
    chunk_size_impl
};

// private functions
static void* chunk_open_impl(const char* filename, const char* mode)
{
    return NULL;
}

static void chunk_close_impl(void* handle)
{
    free(handle);
    return;
}

static int chunk_read_impl(void* handle, void* dst, size_t count)
{
    size_t pos;
    size_t cur_pos, size;
    uint8_t* data;
    chunk_handle_t* hd;

    hd = (chunk_handle_t*)handle;
    cur_pos = hd->pos;
    size = hd->size;

    // EOF
    if (cur_pos >= size)
        return 1;

    pos = cur_pos + count;
    if (pos > size)
        return 2;

    data = hd->data;
    memcpy(dst, data + cur_pos, count);
    hd->pos = pos;
    return 0;
}

static int chunk_write_impl(void* handle, const void* src, size_t count)
{
    return 1;
}

static int chunk_seek_impl(void* handle, long offset, seek_mode_t mode)
{
    long real_offs;
    size_t cur_pos, size;
    chunk_handle_t* hd;

    real_offs = 0;
    hd = (chunk_handle_t*)handle;

    cur_pos = hd->pos;
    size = hd->size;
    switch (mode) {
    case FSEEK_SET:
        real_offs = offset;
        break;
    case FSEEK_CUR:
        real_offs = offset + cur_pos;
        break;
    case FSEEK_END:
        real_offs = offset + size;
        break;
    }

    if (real_offs < 0 || real_offs > size)
        return 1;
    hd->pos = real_offs;
    return 0;
}

static int chunk_pos_impl(void* handle, size_t* pos)
{
    chunk_handle_t* hd;

    hd = (chunk_handle_t*)handle;
    *pos = hd->pos;
    return 0;
}

static void* chunk_dup_impl(void* handle)
{
    return NULL;
}

static size_t chunk_size_impl(void* handle)
{
    chunk_handle_t* hd;

    hd = (chunk_handle_t*)handle;
    return hd->size;
}

// public functions
file_handle_t* chunk_get_data(chunk_t* chunk, size_t idx)
{
    file_handle_t* res;
    chunk_handle_t* chunk_handle;

    if (!chunk)
        return NULL;
    if (idx >= chunk->count)
        return NULL;

    chunk_handle = (chunk_handle_t*)malloc(sizeof(chunk_handle_t));
    if (!chunk_handle)
        return NULL;

    chunk_handle->pos = 0;
    chunk_handle->size = chunk->size[idx];
    chunk_handle->data = chunk->data + chunk->offset[idx];

    res = file_create_custom_handle(chunk_handle, &chunk_handle_op);
    if (!res) {
        free(chunk_handle);
        return NULL;
    }

    return res;
}

int chunk_get_data_count(chunk_t* chunk)
{
    if (!chunk)
        return 0;

    return chunk->count;
}

chunk_t* chunk_open(const char* filename)
{
    chunk_t* res;
    int needed_size;
    size_t chunk_file_size;
    uint8_t data_count;
    file_handle_t* chunk_file;

    uint8_t* p;
    int pos;
    int* size;
    int* offset;

    p = NULL;
    pos = 0;
    chunk_file = file_open(filename, "rb");
    if (!chunk_file)
        return NULL;
    if (file_get_u8(chunk_file, &data_count))
        goto fail;
    if (data_count == 0)
        goto fail;
    chunk_file_size = file_size(chunk_file);
    if (chunk_file_size == 0)
        goto fail;

    needed_size = sizeof(chunk_t) + chunk_file_size - 1;
    p = (uint8_t*)malloc(needed_size);
    if (!p)
        goto fail;

    res = (chunk_t*)p;
    pos += sizeof(chunk_t);
    res->count = data_count;
    res->size = (int*)(p + pos);
    pos += data_count * sizeof(int);
    res->offset = (int*)(p + pos);
    pos += data_count * sizeof(int);
    res->data = p + pos;

    size = res->size;
    offset = res->offset;
    for (int i = 0; i < data_count; i++) {
        uint32_t t[2];
        if (file_get_u32(chunk_file, &t[0]))
            goto fail;
        if (file_get_u32(chunk_file, &t[1]))
            goto fail;

        offset[i] = t[0];
        size[i] = t[1];
    }
    if (file_read(chunk_file, res->data, chunk_file_size - data_count * 8 - 1))
        goto fail;

    file_close(chunk_file);
    return res;

fail:
    if (p)
        free(p);
    file_close(chunk_file);
    return NULL;
}

void chunk_free(chunk_t* chunk)
{
    if (!chunk)
        return;

    free(chunk);
    return;
}

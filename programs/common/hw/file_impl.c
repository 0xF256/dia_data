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

#include "file_impl.h"
#include <stdio.h>
#include <stdlib.h>

// private struct
struct file_handle_s {
    void* handle;
    const file_op_t* op;
};

typedef struct default_handle_impl_s {
    FILE* fp;
    size_t size;
} def_handle_t;

// private functions (public functions statement in header)
static void* file_open_impl(const char* filename, const char* mode);
static void file_close_impl(void* handle);
static int file_read_impl(void* handle, void* dst, size_t count);
static int file_write_impl(void* handle, const void* src, size_t count);
static int file_seek_impl(void* handle, long offset, seek_mode_t mode);
static int file_pos_impl(void* handle, size_t* pos);
static void* file_dup_impl(void* handle);
static size_t file_size_impl(void* handle);

static int file_get_impl(file_handle_t* handle, uint64_t* out, uint8_t count);

// private variables
const static file_op_t default_op = {
    file_open_impl,
    file_close_impl,
    file_read_impl,
    file_write_impl,
    file_seek_impl,
    file_pos_impl,
    file_dup_impl,
    file_size_impl
};

static endian_t global_endian = ENDIAN_LE;
static file_op_t* global_op = (file_op_t*)&default_op;

// function implementation (private)
static void* file_open_impl(const char* filename, const char* mode)
{
    FILE* fp;
    def_handle_t* res;

    fp = fopen(filename, mode);
    if (!fp) return NULL;

    res = (def_handle_t*)malloc(sizeof(def_handle_t));
    if (!res) {
        fclose(fp);
        return NULL;
    }
    // TODO: better way to get size
    fseek(fp, 0, SEEK_END);
    res->size = ftell(fp);
    rewind(fp);

    res->fp = fp;
    return res;
}

static void file_close_impl(void* handle)
{
    def_handle_t* hd;

    hd = (def_handle_t*)handle;
    fclose(hd->fp);
    free(hd);
    return;
}

static int file_read_impl(void* handle, void* dst, size_t count)
{
    def_handle_t* hd;

    hd = (def_handle_t*)handle;
    return !fread(dst, count, 1, hd->fp);
}

static int file_write_impl(void* handle, const void* src, size_t count)
{
    def_handle_t* hd;

    hd = (def_handle_t*)handle;
    return !fwrite(src, count, 1, hd->fp);
}

static int file_seek_impl(void* handle, long offset, seek_mode_t mode)
{
    int _mode;
    def_handle_t* hd;

    hd = (def_handle_t*)handle;
    switch (mode) {
    case FSEEK_SET:
        _mode = SEEK_SET;
        break;
    case FSEEK_CUR:
        _mode = SEEK_CUR;
        break;
    case FSEEK_END:
        _mode = SEEK_END;
        break;
    }

    return !!fseek(hd->fp, offset, _mode);
}

static int file_pos_impl(void* handle, size_t* pos)
{
    long cur_pos;
    def_handle_t* hd;

    hd = (def_handle_t*)handle;
    cur_pos = ftell(hd->fp);
    if (cur_pos <= 0)
        return -1;

    *pos = cur_pos;
    return 0;
}

static void* file_dup_impl(void* handle)
{
    def_handle_t* hd;

    hd = (def_handle_t*)handle;
    // stub
    return NULL;
}

static size_t file_size_impl(void* handle)
{
    def_handle_t* hd;

    hd = (def_handle_t*)handle;
    return hd->size;
}

static int file_get_impl(file_handle_t* handle, uint64_t* out, uint8_t count)
{
    int pos;
    int error;
    uint64_t res;
    uint8_t buf[8];

    res = 0;
    error = file_read(handle, buf, count);
    if (error) return error;

    pos = 0;
    if (global_endian == ENDIAN_LE) {
        for (int i = 0; i < count; i++) {
            res |= (uint64_t)(buf[pos++]) << (i * 8);
        }
    } else {
        while (count--) {
            res |= (uint64_t)(buf[pos++]) << (count * 8);
        }
    }
    *out = res;
    return FILE_SUCCESS;
}

// function implementation (public)
int file_set_global_op(file_op_t* op)
{
    if (!op)
        return FILE_INVALID_PARAM;

    global_op = op;
    return FILE_SUCCESS;
}

file_op_t* file_get_global_op()
{
    return global_op;
}

file_handle_t* file_create_custom_handle(void* handle, const file_op_t* op)
{
    file_handle_t* res;

    if (!handle || !op)
        return NULL;

    res = (file_handle_t*)malloc(sizeof(file_handle_t));
    if (!res) return NULL;

    res->handle = handle;
    res->op = op;
    return res;
}

file_handle_t* file_open(const char* filename, const char* mode)
{
    void* handle;
    file_handle_t* res;

    if (!filename || !mode)
        return NULL;

    handle = global_op->open(filename, mode);
    if (!handle) return NULL;

    res = (file_handle_t*)malloc(sizeof(file_handle_t));
    if (!res) {
        global_op->close(handle);
        return NULL;
    }

    res->handle = handle;
    res->op = global_op;
    return res;
}

void file_close(file_handle_t* handle)
{
    if (!handle)
        return;

    handle->op->close(handle->handle);
    free(handle);
    return;
}

int file_read(file_handle_t* handle, void* dst, size_t count)
{
    if (!handle || !dst || !count)
        return FILE_INVALID_PARAM;
    return handle->op->read(handle->handle, dst, count);
}

int file_write(file_handle_t* handle, const void* src, size_t count)
{
    if (!handle || !src || !count)
        return FILE_INVALID_PARAM;
    return handle->op->write(handle->handle, src, count);
}

int file_seek(file_handle_t* handle, long offset, seek_mode_t mode)
{
    if (!handle || !offset)
        return FILE_INVALID_PARAM;
    return handle->op->seek(handle->handle, offset, mode);
}

int file_pos(file_handle_t* handle, size_t* pos)
{
    if (!handle || !pos)
        return FILE_INVALID_PARAM;
    return handle->op->pos(handle->handle, pos);
}

file_handle_t* file_dup(file_handle_t* handle)
{
    void* handle_dup;
    file_handle_t* res;

    if (!handle)
        return NULL;

    handle_dup = handle->op->dup(handle->handle);
    if (!handle_dup) return NULL;

    res = (file_handle_t*)malloc(sizeof(file_handle_t));
    if (!res) {
        handle->op->close(handle_dup);
        return NULL;
    }

    res->handle = handle_dup;
    res->op = handle->op;
    return res;
}

size_t file_size(file_handle_t* handle)
{
    if (!handle)
        return 0;
    return handle->op->size(handle->handle);
}

void file_set_global_endian(endian_t endian)
{
    global_endian = endian;
    return;
}

endian_t file_get_global_endian()
{
    return global_endian;
}

int file_get_u8(file_handle_t* handle, uint8_t* out)
{
    int error;
    uint64_t temp;

    error = file_get_impl(handle, &temp, 1);
    if (error)
        return error;
    *out = (uint8_t)temp;
    return FILE_SUCCESS;
}

int file_get_u16(file_handle_t* handle, uint16_t* out)
{
    int error;
    uint64_t temp;

    error = file_get_impl(handle, &temp, 2);
    if (error)
        return error;
    *out = (uint16_t)temp;
    return FILE_SUCCESS;
}

int file_get_u24(file_handle_t* handle, uint32_t* out)
{
    int error;
    uint64_t temp;

    error = file_get_impl(handle, &temp, 3);
    if (error)
        return error;
    *out = (uint32_t)temp;
    return FILE_SUCCESS;
}

int file_get_u32(file_handle_t* handle, uint32_t* out)
{
    int error;
    uint64_t temp;

    error = file_get_impl(handle, &temp, 4);
    if (error)
        return error;
    *out = (uint32_t)temp;
    return FILE_SUCCESS;
}

int file_get_u64(file_handle_t* handle, uint64_t* out)
{
    int error;
    uint64_t temp;

    error = file_get_impl(handle, &temp, 8);
    if (error)
        return error;
    *out = temp;
    return FILE_SUCCESS;
}

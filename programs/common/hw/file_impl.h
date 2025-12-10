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

#ifndef _IO_IMPL_H_
#define _IO_IMPL_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// defines
#define FILE_SUCCESS        0
#define FILE_INVALID_PARAM -1

// structs
typedef enum
{
    ENDIAN_LE,
    ENDIAN_BE
} endian_t;

typedef enum
{
    FSEEK_CUR,
    FSEEK_SET,
    FSEEK_END
} seek_mode_t;

typedef struct file_op_s
{
    void*  (*open)(const char* filename, const char *mode);
    void   (*close)(void *handle);
    int    (*read)(void *handle, void *dst, size_t count);
    int    (*write)(void *handle, const void *src, size_t count);
    int    (*seek)(void *handle, long offset, seek_mode_t mode);
    int    (*pos)(void *handle, size_t *pos);
    void*  (*dup)(void *handle);
    size_t (*size)(void *handle);
} file_op_t;

struct file_handle_s;
typedef struct file_handle_s file_handle_t;

// public functions
int              file_set_global_op(file_op_t *op);
file_op_t*       file_get_global_op();

file_handle_t*   file_create_custom_handle(void *handle, const file_op_t *op);

file_handle_t*  file_open(const char* filename, const char *mode);
void            file_close(file_handle_t *handle);
int             file_read(file_handle_t *handle, void *dst, size_t count);
int             file_write(file_handle_t *handle, const void *src, size_t count);
int             file_seek(file_handle_t *handle, long offset, seek_mode_t mode);
int             file_pos(file_handle_t *handle, size_t *pos);
file_handle_t*  file_dup(file_handle_t *handle);
size_t          file_size(file_handle_t *handle);

void     file_set_global_endian(endian_t endian);
endian_t file_get_global_endian();

int file_get_u8(file_handle_t *handle, uint8_t *out);
int file_get_u16(file_handle_t *handle, uint16_t *out);
int file_get_u24(file_handle_t *handle, uint32_t *out);
int file_get_u32(file_handle_t *handle, uint32_t *out);
int file_get_u64(file_handle_t *handle, uint64_t *out);

#ifdef __cplusplus
}
#endif

#endif
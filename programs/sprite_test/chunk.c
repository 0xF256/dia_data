#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define GEN_READ(A)                                         \
    int chunk_read_##A(chunk_t *chunk, A *dest)             \
    {                                                       \
        size_t cur_pos;                                     \
        if(!chunk || !dest) return 1;                       \
        cur_pos = chunk->cur_pos;                           \
        if(cur_pos + sizeof(*dest) > chunk->size) return 2; \
        *dest = *((typeof(dest))(&(chunk->data[cur_pos]))); \
        chunk->cur_pos += sizeof(*dest);                    \
        return 0;                                           \
    }                                                       \

typedef uint8_t u8;
typedef uint16_t u16;

enum
{
    POS_OFFSET = 0,
    POS_SIZE,
};

typedef union
{
    uint32_t pos[2];
} chunk_size_t;

typedef struct
{
    size_t size;
    size_t cur_pos;
    uint8_t data[0];
} chunk_t;

GEN_READ(u8)
GEN_READ(u16)

int chunk_rewind(chunk_t *chunk)
{
    if(!chunk) return 1;
    chunk->cur_pos = 0;
    return 0;
}

int chunk_skip_bytes(chunk_t *chunk, size_t count)
{
    if(!chunk || !count) return 1;
    if(count > chunk->size || chunk->cur_pos + count > chunk->size) return 2;
    chunk->cur_pos += count;
    return 0;
}

int chunk_read_bytes(chunk_t *chunk, uint8_t *dest, size_t count)
{
    uint8_t *src;
    size_t cur_pos;
    // Invalid param
    if(!chunk || !dest || !count) return 1;

    cur_pos = chunk->cur_pos;
    // 为了防止溢出
    if(count > chunk->size || cur_pos + count > chunk->size) return 2;
    src = &(chunk->data[cur_pos]);
    chunk->cur_pos += count;

    for(size_t i = 0; i < count; i++)
        dest[i] = src[i];
    return 0;
}

void chunk_free(chunk_t *chunk)
{
    free(chunk);
}

chunk_t *chunk_load(const char *filename, int chunk_index)
{
    int total_chunk;
    FILE *fp = NULL;
    chunk_t *res = NULL;
    chunk_size_t *size_info = NULL;

    if(!filename || chunk_index < 0)
    {
        return NULL;
    }

    fp = fopen(filename, "rb");
    if(!fp) return NULL;

    total_chunk = fgetc(fp);
    if(total_chunk <= 0 || chunk_index >= total_chunk)
    {
        goto fail;
    }

    size_info = (chunk_size_t*)malloc(total_chunk * sizeof(chunk_size_t));
    if(!size_info || fread(size_info, sizeof(chunk_size_t), total_chunk, fp) != total_chunk)
    {
        goto fail;
    }

    if(fseek(fp, size_info[chunk_index].pos[POS_OFFSET], SEEK_CUR)) goto fail;
    if(!(res = (chunk_t*)malloc(size_info[chunk_index].pos[POS_SIZE] + sizeof(chunk_t)))) goto fail;
    res->cur_pos = 0;
    res->size = size_info[chunk_index].pos[POS_SIZE];
    if(!fread(res->data, res->size, 1, fp)) goto fail;

    fclose(fp);
    free(size_info);
    return res;
fail:
    if(fp) fclose(fp);
    if(res) free(res);
    if(size_info) free(size_info);
    return NULL;
}

int chunk_save(const char *filename, chunk_t *chunk)
{
    if(!filename || !chunk) return 1;

    FILE *fp = fopen(filename, "wb");
    if(!fp) return -1;

    if(!fwrite(chunk->data, chunk->size, 1, fp)) goto fail;

    fflush(fp);
    fclose(fp);
    return 0;
fail:
    fclose(fp);
    return -1;
}

#define CHUNK_TEST 0
#if CHUNK_TEST

int main(int argc, const char **argv)
{
    if(argc < 4) return -1;

    const char *chunk_filename = argv[1];
    int index = atoi(argv[2]);
    const char *save_name = argv[3];
    chunk_t *chunk = NULL;

    if(!(chunk = chunk_load(chunk_filename, index)))
    {
        fprintf(stderr, "Failed to load \"%s\"\n", chunk_filename);
        return -1;
    }

    if(chunk_save(save_name, chunk))
    {
        fprintf(stderr, "Failed to save \"%s\"\n", save_name);
        return -1;
    }

    free(chunk);
    return 0;
}

#endif

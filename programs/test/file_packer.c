#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define HELP \
    "Usage: packer [OUT_FILE] [IN_FILES]\n\n" \
    "A tool to pack file(s) into DiamondRush chunk file\n"

int help()
{
    fprintf(stderr, HELP);
    return -1;
}

int main(int argc, const char **argv)
{
    int res = 1;

    FILE *out;
    FILE **in;
    size_t *size;
    size_t offset = 0;

    int in_count = argc - 2;
    char buf[4096];

    if(argc < 3) return help();

    out = fopen(argv[1], "wb");
    if(!out)
    {
        fprintf(stderr, "Failed to create file: %s\n", strerror(errno));
        return 1;
    }

    in = (FILE**)calloc(in_count, sizeof(FILE*));
    size = (size_t*)calloc(in_count, sizeof(size_t));
    if(!in) goto fail1;
    if(!size) goto fail2;
    for(int i = 0; i < in_count; i++)
    {
        in[i] = fopen(argv[i+2], "rb");
        if(!in[i]) goto fail2;
        fseek(in[i], 0, SEEK_END);
        size[i] = ftell(in[i]);
        rewind(in[i]);
    }

    fputc(in_count, out);
    for(int i = 0; i < in_count; i++)
    {
        fwrite(&offset, sizeof(uint32_t), 1, out);
        fwrite(&size[i], sizeof(uint32_t), 1, out);
        offset += size[i];
    }

    for(int i = 0; i < in_count; i++)
    {
        size_t read_count;
        while((read_count = fread(buf, sizeof(char), 4096, in[i])))
        {
            fwrite(buf, sizeof(char), read_count, out);
        }
    }
    fflush(out);

    res = 0;

    free(size);
fail2:
    for(int i = 0; i < in_count; i++)
    {
        if(!in[i]) break;
        fclose(in[i]);
    }
    free(in);
fail1:
    fclose(out);
    return res;
}

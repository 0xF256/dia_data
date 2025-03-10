#if !defined(WINNT) && defined(__unix__)

#include <zlib.h>
#include <stdlib.h>
static int _crc32(unsigned char *buffer, int len)
{
    return crc32(0, buffer, len);
}
static unsigned char *_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality)
{
    // quick and dirty
    unsigned long out_len_ = compressBound(data_len);
    unsigned char *data_out = malloc(out_len_);

    if(data_out != NULL)
    {
        if(compress2(data_out, &out_len_, data, data_len, quality) == Z_OK)
        {
            *out_len = (int)out_len_;
            return data_out;
        }
        else
        {
            // TODO: handle compression error?
            free(data_out);
            return NULL;
        }
    }
    else
    {
        // Out of memory?
        return NULL;
    }
}
#define STBIW_CRC32 _crc32
#define STBIW_ZLIB_COMPRESS _zlib_compress

#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include "stb_image_write.h"

int image_save_png(char const *filename, int x, int y, int comp, const void *data, int stride_bytes)
{
    return !stbi_write_png(filename, x, y, comp, data, stride_bytes);
}

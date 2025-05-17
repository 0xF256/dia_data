#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TAB "    "

#define STRINGIFY(s) #s
#define RANGE(x, y, z) if(x >= y && x <= z)

typedef uint64_t ull;

enum
{
    BS_MODULES              = (1 <<  0),
    BS_MODULES_XY           = (1 <<  1),
    BS_MODULES_IMG          = (1 <<  2),
    BS_MODULE_IMAGES_TC_BMP = (1 <<  3),
    BS_MODULES_WH_SHORT     = (1 <<  4),
    BS_MODULES_XY_SHORT     = (1 <<  5),
    BS_MODULES_USAGE        = (1 <<  6),
    BS_IMAGE_SIZE_INT       = (1 <<  7),
    BS_FRAMES               = (1 <<  8),
    BS_FM_OFF_SHORT         = (1 << 10),
    BS_NFM_SHORT            = (1 << 11),
    BS_SKIP_FRAME_RC        = (1 << 12),
    BS_FRAME_COLL_RC        = (1 << 13),
    BS_FM_PALETTE           = (1 << 14),
    BS_FRAME_RECTS          = (1 << 15),
    BS_ANIMS                = (1 << 16),
    BS_NO_AF_START          = (1 << 17),
    BS_AF_OFF_SHORT         = (1 << 18),
    BS_NAF_SHORT            = (1 << 19),
    BS_FM_INDEX_SHORT       = (1 << 20),
    BS_AF_INDEX_SHORT       = (1 << 21),
    BS_EXTRA_FLAGS          = (1 << 22),
    BS_MODULE_IMAGES_FX     = (1 << 23),
    BS_MODULE_IMAGES        = (1 << 24),
    BS_PNG_CRC              = (1 << 25),
    BS_KEEP_PAL             = (1 << 26),
    BS_TRANSP_FIRST         = (1 << 27),
    BS_TRANSP_LAST          = (1 << 28),
    BS_SINGLE_IMAGE         = (1 << 29),
    BS_MULTIPLE_IMAGES      = (1 << 30),
    BS_GIF_HEADER           = (1 << 31)
};

const char *bs_flag_str[] =
{
    STRINGIFY(BS_MODULES),
    STRINGIFY(BS_MODULES_XY),
    STRINGIFY(BS_MODULES_IMG),
    STRINGIFY(BS_MODULE_IMAGES_TC_BMP),
    STRINGIFY(BS_MODULES_WH_SHORT),
    STRINGIFY(BS_MODULES_XY_SHORT),
    STRINGIFY(BS_MODULES_USAGE),
    STRINGIFY(BS_IMAGE_SIZE_INT),
    STRINGIFY(BS_FRAMES),
    NULL,
    STRINGIFY(BS_FM_OFF_SHORT),
    STRINGIFY(BS_NFM_SHORT),
    STRINGIFY(BS_SKIP_FRAME_RC),
    STRINGIFY(BS_FRAME_COLL_RC),
    STRINGIFY(BS_FM_PALETTE),
    STRINGIFY(BS_FRAME_RECTS),
    STRINGIFY(BS_ANIMS),
    STRINGIFY(BS_NO_AF_START),
    STRINGIFY(BS_AF_OFF_SHORT),
    STRINGIFY(BS_NAF_SHORT),
    STRINGIFY(BS_FM_INDEX_SHORT),
    STRINGIFY(BS_AF_INDEX_SHORT),
    STRINGIFY(BS_EXTRA_FLAGS),
    STRINGIFY(BS_MODULE_IMAGES_FX),
    STRINGIFY(BS_MODULE_IMAGES),
    STRINGIFY(BS_PNG_CRC),
    STRINGIFY(BS_KEEP_PAL),
    STRINGIFY(BS_TRANSP_FIRST),
    STRINGIFY(BS_TRANSP_LAST),
    STRINGIFY(BS_SINGLE_IMAGE),
    STRINGIFY(BS_MULTIPLE_IMAGES),
    STRINGIFY(BS_GIF_HEADER)
};

ull hex2digit(const char *str)
{
    ull n = 1;
    ull res = 0;
    for(int i = strlen(str) - 1; i >= 0; i--)
    {
        int t;
        RANGE(str[i], 'a', 'f')
            t = str[i] - 'a' + 10;
        else RANGE(str[i], 'A', 'F')
            t = str[i] - 'A' + 10;
        else RANGE(str[i], '0', '9')
            t = str[i] - '0';
        else
            continue;

        res += t * n;
        n *= 16;
    }
    return res;
}

void print_bs_flags(ull flags)
{
    for(int i = 0; i < sizeof(bs_flag_str)/sizeof(bs_flag_str[0]); i++)
    {
        if(flags & ((ull)1 << i))
        {
            printf(TAB"%s %d\n", bs_flag_str[i], i);
        }
    }
    return;
}

int main(int argc, const char **argv)
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: bs_flag_dec [HEX]...\n");
        return -1;
    }

    for(int i = 1; i < argc; i++)
    {
        ull flags = hex2digit(argv[i]);

        printf("BS_FLAG(0x%08X)\n", (uint32_t)flags);
        print_bs_flags(flags);
    }
    return 0;
}
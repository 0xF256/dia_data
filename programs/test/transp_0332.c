#include <stdio.h>
#include <stdint.h>

union
{
    uint32_t argb;
    uint8_t  channel[4];
} color;

int main()
{
    uint8_t c = 0xC0;
    color.argb = (0xFF000000 | ((c & 0xE0) << 16) | ((c & 0x1C) << 11) | ((c & 0x3) << 6));
    printf("#%02x%02x%02x\n", color.channel[2], color.channel[1], color.channel[0]);
    printf("RGB(%d,%d,%d)\n", color.channel[2], color.channel[1], color.channel[0]);
    return 0;
}
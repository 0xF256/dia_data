#include <stdio.h>
#include <stdint.h>

union
{
    uint32_t argb;
    uint8_t  channel[4];
} color;

int main()
{
    uint16_t c = 0xF81F;
    color.argb = (0xFF000000 | (c & 0xF800) << 8 | (c & 0x7E0) << 5 | (c & 0x1F) << 3);
    printf("#%02x%02x%02x\n", color.channel[2], color.channel[1], color.channel[0]);
    printf("RGB(%d,%d,%d)\n", color.channel[2], color.channel[1], color.channel[0]);
    return 0;
}

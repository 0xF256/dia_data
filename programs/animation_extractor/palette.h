#include <stdint.h>

#ifndef _PALETTE_H_
#define _PALETTE_H_

struct palette_s;
typedef struct palette_s palette_t;

struct palettes_s;
typedef struct palettes_s palettes_t;

int palette_get_count(const palettes_t *palettes);
palette_t *palette_get(palettes_t *palettes, size_t index);
uint32_t palette_get_color(const palette_t *palette, size_t color_index);
void palettes_free(palettes_t *palettes);
palettes_t *palette_load(const uint8_t *data, uint16_t pixel_format, uint8_t total_palette, uint8_t total_pixels);

#endif

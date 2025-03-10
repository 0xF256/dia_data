#include <stdint.h>
#include "palette.h"

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

struct texture_s;
typedef struct texture_s texture_t;

int texture_get_width(const texture_t *tex);
int texture_get_height(const texture_t *tex);
const uint32_t *texture_get_pixels(const texture_t *tex);
void texture_free(texture_t *texture);
texture_t *texture_load(const uint8_t *decode_data, const uint16_t decode_len, const uint16_t decode_type, const palette_t *palette, int w, int h, int scale);

#endif

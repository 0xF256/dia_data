#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

#include <SDL2/SDL.h>

struct graphic_s;
typedef struct graphic_s graphic_t;

struct tex_s;
typedef struct tex_s tex_t;

int graphic_init(const char *window_name, int width, int height);

int graphic_get_width();

int graphic_get_height();

void graphic_show_window();

void graphic_draw_region(tex_t *texture, int x, int y, int transform);

void graphic_present();

void graphic_quit();

tex_t *graphic_create_texture(const void *pixels, int w, int h, int pitch);

void graphic_destroy_texture(tex_t *tex);

int graphic_take_screenshot(const char *filename);

#endif

#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

#include <SDL2/SDL.h>

struct graphic_s;
typedef struct graphic_s graphic_t;

struct tex_s;
typedef struct tex_s tex_t;

int graphic_init(const char *window_name, int width, int height, int render_target);

int graphic_get_width();

int graphic_get_height();

void graphic_show_window();

void graphic_draw_region(tex_t *texture, int x, int y, int transform);

void graphic_clear();

void graphic_present();

void graphic_quit();

int graphic_get_texture_width(tex_t *tex);

int graphic_get_texture_height(tex_t *tex);

tex_t *graphic_create_texture(const void *pixels, int w, int h, int pitch);

void graphic_destroy_texture(tex_t *tex);

int graphic_take_screenshot(const char *filename);

int graphic_take_region_screenshot(const char *filename, int x, int y, int w, int h);

#endif

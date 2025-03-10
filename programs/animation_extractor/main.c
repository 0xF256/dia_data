#include <stdio.h>
#include <stdlib.h>
#include "chunk.h"
#include "sprite.h"
#include "graphic.h"
#include "texture.h"

#define CANVAS_SIZE 2000

#define ABOUT \
    "Animation eXtractor v1.0 built on " __DATE__ " " __TIME__ ".\n" \
    "Copyright (c) 2025 - written by SilentTalk.\n" \
    "Licensed under MIT. See source distribution for detailed\n" \
    "copyright notices.\n\n"

int scale;
int frames_count = 0;
int no_tile_pos_info = 0;

tex_t **textures;
int texture_count;
sprite_t *animation;

struct options_s
{
    int scale;
    int chunk_index;
    int palette_index;
    const char *animation_src;
} options = {
    3, -1, 0, NULL
};

#define COUNT_CHECK() if(++i == argc) goto fail
#define INVALID_ARG(x) fprintf(stderr, "Info: Invalid %s, using default setting.\n", #x)
int get_options(struct options_s *opts, int argc, const char **argv)
{
    // if(!opts) { exit(-1); }
    int i;
    int tmp;
    for(i = 1; i < argc; i++)
    {
        if(strlen(argv[i]) == 2 && argv[i][0] == '-')
        {
            switch(argv[i][1])
            {
                // scale
                case 's':
                    COUNT_CHECK();
                    tmp = atoi(argv[i]);
                    if(tmp > 0 && tmp <= 5)
                        opts->scale = tmp;
                    else
                        INVALID_ARG(scale);
                    break;

                // chunk index
                case 'c':
                    COUNT_CHECK();
                    tmp = atoi(argv[i]);
                    if(tmp >= 0)
                        opts->chunk_index = tmp;
                    else
                    {
                        fprintf(stderr, "Error: Invalid chunk index\n");
                        return -1;
                    }
                    break;

                // palette index
                case 'p':
                    COUNT_CHECK();
                    tmp = atoi(argv[i]);
                    if(tmp >= 0)
                        opts->palette_index = tmp;
                    else
                        INVALID_ARG(palette_index);
                    break;

                default:
                    fprintf(stderr, "%s: invalid option -- %c\n", argv[0], argv[i][1]);
                    return -1;
            }
        } else
            opts->animation_src = argv[i];
    }

    if(opts->chunk_index < 0)
    {
        fprintf(stderr, "Error: No chunk index to be used.\n");
        return -1;
    }
    if(!(opts->animation_src))
    {
        fprintf(stderr, "Error: No file to be used.\n");
        return -1;
    }
    return 0;

fail:
    i--;
    fprintf(stderr, "%s: option requires an argument -- %c\n", argv[0], argv[i][1]);
    return -1;
}

void print_help()
{
    fprintf(stderr, ABOUT);
    fprintf(stderr, "Usage: animation_extractor [-d] [-f FPS] [-s SCALE] [-b BG_INDEX] [-c CHUNK_INDEX] [-p PALETTE_INDEX] [FILE]\n\n");
    fprintf(stderr, "Extract animations from DiamondRush's file\n\n");
    fprintf(stderr, "  -s SCALE          Rescale the image\n");
    fprintf(stderr, "  -c CHUNK_INDEX    Specify the chunk to extract\n");
    fprintf(stderr, "  -p PALETTE_INDEX  Specify the palette to be used\n");
    return;
}

int init_res(const char *animation_src, int chunk_index, int palette_index)
{
    chunk_t *animation_chunk;
    animation_chunk = chunk_load(animation_src, chunk_index);

    animation = sprite_load(animation_chunk, scale);
    if(!animation)
    {
        fprintf(stderr, "Failed to load file\n");
        goto fail1;
    }

    texture_count = sprite_get_texture_count(animation);
    textures = (tex_t**)calloc(texture_count, sizeof(tex_t*));
    if(!textures) goto fail1;
    for(int i = 0; i < texture_count; i++)
    {
        texture_t *texture = sprite_get_texture(animation, i, palette_index);
        if(!texture) goto fail2;

        int w = texture_get_width(texture);
        int h = texture_get_height(texture);
        const uint32_t *pixels = texture_get_pixels(texture);
        textures[i] = graphic_create_texture(pixels, w, h, w * 4);

        texture_free(texture);
        if(!textures[i]) goto fail2;
    }

    chunk_free(animation_chunk);
    return 0;

fail2:
    for(int i = 0; i < texture_count; i++)
    {
        if(!textures[i]) break;
        graphic_destroy_texture(textures[i]);
    }
    free(textures);
fail1:
    chunk_free(animation_chunk);
    return -1;
}

void free_res()
{
    for(int i = 0; i < texture_count; i++)
    {
        graphic_destroy_texture(textures[i]);
    }
    free(textures);

    sprite_free(animation);
    graphic_quit();
    return;
}

int top_left_vertex_x, top_left_vertex_y;
int bottom_right_vertex_x, bottom_right_vertex_y;
void draw_and_store_image_size(tex_t *tex, int x, int y, int transform)
{
    int tmp_x, tmp_y;

    // TODO: width and height check
    tmp_x = x + graphic_get_texture_width(tex);
    tmp_y = y + graphic_get_texture_height(tex);

    if(x < top_left_vertex_x) top_left_vertex_x = x;
    if(y < top_left_vertex_y) top_left_vertex_y = y;

    if(tmp_x > bottom_right_vertex_x) bottom_right_vertex_x = tmp_x;
    if(tmp_y > bottom_right_vertex_y) bottom_right_vertex_y = tmp_y;

    graphic_draw_region(tex, x, y, transform);
    return;
}

void clear_image_size()
{
    top_left_vertex_x = top_left_vertex_y = CANVAS_SIZE;
    bottom_right_vertex_x = bottom_right_vertex_y = 0;
    return;
}

void sprite_paint_tiles_image(sprite_t *sprite, int pos_info_index, int x, int y)
{
    static int init = 0;

    int count, offset;
    static size_t total_pos_info;

    static tile_pos_t *tile_pos;
    static pos_info_t *tile_pos_info;

    if(!sprite) return;
    if(pos_info_index < 0) return;

    if(!init)
    {
        tile_pos = sprite_get_tile_pos(sprite, NULL);
        tile_pos_info = sprite_get_tile_pos_info(sprite, &total_pos_info);

        init = 1;
    }

    if(total_pos_info <= 0 || total_pos_info <= pos_info_index) return;

    count = tile_pos_info[pos_info_index].count;
    offset = tile_pos_info[pos_info_index].offset;

    int tex_index, tile_x, tile_y, transform;
    for(int i = 0; i < count; i++)
    {
        tex_index = tile_pos[i+offset].tex_index;
        tile_x = tile_pos[i+offset].x;
        tile_y = tile_pos[i+offset].y;
        transform = tile_pos[i+offset].transform;

        draw_and_store_image_size(textures[tex_index], x + tile_x, y + tile_y, transform);
    }
    return;
}

void draw_animation()
{
    if(no_tile_pos_info)
    {
        draw_and_store_image_size(textures[frames_count], CANVAS_SIZE/2, CANVAS_SIZE/2, 0);
        return;
    }

    sprite_paint_tiles_image(animation, frames_count, CANVAS_SIZE/2, CANVAS_SIZE/2);
    return;
}

int main(int argc, const char **argv)
{
    if(get_options(&options, argc, argv))
    {
        print_help();
        return -1;
    }
    scale = options.scale;

    if(graphic_init("DiamondRush Animation eXtractor", CANVAS_SIZE, CANVAS_SIZE, 1))
    {
        fprintf(stderr, "Failed to init graphic.\n");
        return -1;
    }

    if(init_res(options.animation_src, options.chunk_index, options.palette_index))
    {
        fprintf(stderr, "Failed to load resources.\n");
        return -1;
    }

    int total_image = sprite_get_tile_pos_info_count(animation);
    if(!total_image)
    {
        no_tile_pos_info = 1;
        if(!(total_image = sprite_get_texture_count(animation)))
        {
            fprintf(stderr, "Erorr: No images found in file.\n");
            return -1;
        }
    }

    {
        char buf[128];
        for(int i = 0; i < total_image; i++)
        {
            frames_count = i;
        #ifdef WINNT
            sprintf(buf, "save\\images_dump_%03d.png", i+1);
        #else
            sprintf(buf, "save/images_dump_%03d.png", i+1);
        #endif
            clear_image_size();
            graphic_clear();

            draw_animation();

            graphic_present();
            if(!graphic_take_region_screenshot(buf, top_left_vertex_x, top_left_vertex_y, bottom_right_vertex_x - top_left_vertex_x, bottom_right_vertex_y - top_left_vertex_y))
            {
                fprintf(stderr, "Info: [%s] saved\n", buf);
                printf("index: %d, pos_x: %d, pos_y: %d\n", i, top_left_vertex_x - CANVAS_SIZE/2, top_left_vertex_y - CANVAS_SIZE/2);
            }
            else
            {
                fprintf(stderr, "%s\n", SDL_GetError());
            }
        }
    }

    free_res();
    return 0;
}
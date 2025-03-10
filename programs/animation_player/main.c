#include <stdio.h>
#include <stdlib.h>
#include "chunk.h"
#include "sprite.h"
#include "graphic.h"
#include "texture.h"

#include "background_data.h"

#define ABOUT \
    "DiamondRush Animation Player v1.0 built on " __DATE__ " " __TIME__ ".\n" \
    "Copyright (c) 2025 - written by SilentTalk.\n" \
    "Licensed under MIT. See source distribution for detailed\n" \
    "copyright notices.\n\n"

int scale;
int frames_per_sec;
int frames_count = 0;
int display_single_image = 0;

tex_t *background = NULL;

tex_t **textures;
int texture_count;
sprite_t *animation;

struct options_s
{
    int scale;
    int bg_index;
    int dump_frames;
    int chunk_index;
    int palette_index;
    int frames_per_sec;
    const char *animation_src;
} options = {
    3, 0, 0, -1, 0, 8, NULL
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

                // background index
                case 'b':
                    COUNT_CHECK();
                    tmp = atoi(argv[i]);
                    if(tmp >= 0 && tmp < 3)
                        opts->bg_index = tmp;
                    else
                        INVALID_ARG(background);
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

                // fps
                case 'f':
                    COUNT_CHECK();
                    tmp = atoi(argv[i]);
                    if(tmp > 0 && tmp <= 60)
                        opts->frames_per_sec = tmp;
                    else
                        INVALID_ARG(fps);
                    break;

                // dump frames
                case 'd':
                    opts->dump_frames = 1;
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
    fprintf(stderr, "Usage: animation_player [-d] [-f FPS] [-s SCALE] [-b BG_INDEX] [-c CHUNK_INDEX] [-p PALETTE_INDEX] [FILE]\n\n");
    fprintf(stderr, "Play animations from DiamondRush's file\n\n");
    fprintf(stderr, "  -d                Dump all animation frames\n");
    fprintf(stderr, "  -f FPS            Set player FPS\n");
    fprintf(stderr, "  -s SCALE          Rescale the image\n");
    fprintf(stderr, "  -b BG_INDEX       Set background index (default: 0)\n");
    fprintf(stderr, "  -c CHUNK_INDEX    Specify the chunk to extract\n");
    fprintf(stderr, "  -p PALETTE_INDEX  Specify the palette to be used\n");
    return;
}

int load_background_texture(int index)
{
    int res = -1;
    // if(index < 0 || index > 3) return -1;
    texture_t *texture;
    palettes_t *palettes;

    uint8_t total_color = bg[index].total_color;
    const uint8_t *decode_data = bg[index].decode_data;
    const uint8_t *palette_data = bg[index].palette_data;

    palettes = palette_load(palette_data, 0x5515, 1, total_color);
    if(!palettes) return 1;

    texture = texture_load(decode_data, 0x120, 0x1600, palette_get(palettes, 0), 24, 24, scale);
    if(!texture) goto fail1;

    background = graphic_create_texture(texture_get_pixels(texture), 24 * scale, 24 * scale, 24 * scale * 4);
    if(!background) goto fail2;

    res = 0;
fail2:
    texture_free(texture);
fail1:
    palettes_free(palettes);
    return res;
}

int init_res(const char *animation_src, int chunk_index, int palette_index, int bg_index)
{
    chunk_t *animation_chunk;
    animation_chunk = chunk_load(animation_src, chunk_index);

    animation = sprite_load(animation_chunk, scale);
    if(!animation || load_background_texture(bg_index))
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
    graphic_destroy_texture(background);
    return -1;
}

void free_res()
{
    for(int i = 0; i < texture_count; i++)
    {
        graphic_destroy_texture(textures[i]);
    }
    free(textures);
    graphic_destroy_texture(background);

    sprite_free(animation);
    graphic_quit();
    return;
}

void draw_background()
{
    for(int y = 0; y < 11; y++)
    {
        for(int x = 0; x < 11; x++)
        {
            graphic_draw_region(background, x * 24 * scale, y * 24 * scale, 0);
        }
    }
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

        graphic_draw_region(textures[tex_index], x + tile_x, y + tile_y, transform);
    }
    return;
}

void draw_animation()
{
    if(display_single_image)
    {
        graphic_draw_region(textures[frames_count], 24 * 5 * scale, 24 * 5 * scale, 0);
        return;
    }

    sprite_paint_tiles_image(animation, frames_count, 24 * 5 * scale, 24 * 5 * scale);
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
    frames_per_sec = options.frames_per_sec;

    if(graphic_init("DiamondRush Animation Player", 24 * 11 * scale, 24 * 11 * scale))
    {
        fprintf(stderr, "Failed to init graphic.\n");
        return -1;
    }

    if(init_res(options.animation_src, options.chunk_index, options.palette_index, options.bg_index))
    {
        fprintf(stderr, "Failed to load resources.\n");
        return -1;
    }

    int total_image = sprite_get_tile_pos_info_count(animation);
    if(!total_image)
    {
        display_single_image = 1;
        if(!(total_image = sprite_get_texture_count(animation)))
        {
            fprintf(stderr, "Erorr: No images found in file.\n");
            return -1;
        }
        fprintf(stderr, "Info: Player will display single image.\n");
    }

    if(options.dump_frames)
    {
        char buf[128];
        for(int i = 0; i < total_image; i++)
        {
            frames_count = i;
        #ifdef WINNT
            sprintf(buf, "save\\frames_dump_%03d.png", i+1);
        #else
            sprintf(buf, "save/frames_dump_%03d.png", i+1);
        #endif
            draw_background();
            draw_animation();
            graphic_present();
            if(!graphic_take_screenshot(buf))
            {
                fprintf(stderr, "Info: [%s] saved\n", buf);
            }
            else
            {
                fprintf(stderr, "%s\n", SDL_GetError());
            }
        }
        goto release_res;
    }

    graphic_show_window();

    SDL_Event event;
    SDL_bool running = SDL_TRUE;
    while(1)
    {
        uint32_t begin = SDL_GetTicks();
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    running = SDL_FALSE;
                default:
                    break;
            }
        }
        if(!running) break;

        draw_background();
        draw_animation();

        graphic_present();

        uint32_t current = SDL_GetTicks();
        uint32_t cost = current - begin;
        long delay = (1000/frames_per_sec) - cost;
        if(delay > 0)
        {
            SDL_Delay(delay);
        }

        frames_count = (frames_count + 1) % total_image;
    }

release_res:
    free_res();
    return 0;
}
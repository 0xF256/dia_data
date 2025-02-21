#include <stdlib.h>
#include <string.h>
#include "chunk.h"
#include "graphic.h"
#include "sprite.h"

#include "background_data.h"

// For animation use
int scale;
int frames_per_sec;
int frames_count = 0;
int display_single_image = 0;

sprite_t *animation;
sprite_t *background;

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

#define COUNT_CHECK() if(i++ == argc - 1) goto fail
#define INVALID_ARG(x) fprintf(stderr, "Info: Invalid %s, using default setting.\n", #x)
int get_options(struct options_s *opts, int argc, const char **argv)
{
    // 这里就不需要加这个了
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
                    if(tmp > 0 && tmp < 5)
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
                    if(tmp > 0)
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
                    fprintf(stderr, "%s: invalid option -- %c\n", argv[0], argv[i][0]);
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
    fprintf(stderr, "%s: option requires an argument -- %c\n", argv[0], argv[i][0]);
    return -1;
}

int init_res(graphic_t *graphic, const char *animation_src, int chunk_index, int palette_index, int scale, int bg_index)
{
    chunk_t *animation_chunk;
    // Animation chunk load
    animation_chunk = chunk_load(animation_src, chunk_index);

    animation = sprite_load(animation_chunk->data, scale, graphic);
    sprite_set_cur_palette(animation, palette_index);
    background = sprite_load(backgrounds[bg_index], scale, graphic);
    if(!animation || !background) return 1;

    free(animation_chunk);
    return 0;
}

void draw_background(graphic_t *graphic)
{
    for(int y = 0; y < 11; y++)
    {
        for(int x = 0; x < 11; x++)
        {
            sprite_paint_single_image(background, graphic, 0, x * 24 * scale, y * 24 * scale);
        }
    }
    return;
}

void draw_animation(graphic_t *graphic)
{
    if(display_single_image)
        sprite_paint_single_image(animation, graphic, frames_count, 24 * scale * 5, 24 * scale * 5);
    else
        sprite_paint_tiles_image(animation, graphic, frames_count, 24 * scale * 5, 24 * scale * 5);
    return;
}

int main(int argc, const char **argv)
{
    graphic_t *graphic;

    if(get_options(&options, argc, argv))
    {
        // help();
        return -1;
    }
    scale = options.scale;
    frames_per_sec = options.frames_per_sec;

    if(graphic_init(&graphic, "DiamondRush Animation Player", 24 * scale * 11, 24 * scale * 11))
    {
        fprintf(stderr, "Error: Failed to init graphic.\n");
        return -1;
    }

    if(init_res(graphic, options.animation_src, options.chunk_index, options.palette_index, options.scale, options.bg_index))
    {
        fprintf(stderr, "Error: Failed to init resources.\n");
        return -1;
    }

    int total_image = sprite_get_tile_pos_info_count(animation);
    if(!total_image)
    {
        display_single_image = 1;
        if(!(total_image = sprite_get_tile_count(animation)))
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
            draw_background(graphic);
            draw_animation(graphic);
            graphic_present(graphic);
            if(!graphic_take_screenshot(graphic, buf))
            {
                fprintf(stderr, "Info: [%s] saved\n", buf);
            }
        }
        goto release_res;
    }

    graphic_show_window(graphic);

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

        draw_background(graphic);
        draw_animation(graphic);

        graphic_present(graphic);

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
    sprite_free(background);
    sprite_free(animation);
    graphic_quit(graphic);
    return 0;
}
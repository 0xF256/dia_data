#include <stdio.h>
#include <stdlib.h>
#include "sprite.h"
#include "image_save.h"

void print_tex_info(sprite_t *sprite, size_t count)
{
    dim_t *dims = sprite_get_dimensions(sprite);
    if(!dims)
    {
        fprintf(stderr, "Failed to get dimensions\n");
        return;
    }

    for(int i = 0; i < count; i++)
    {
        printf("textures[%d]: %d x %d\n", i, dims[i].w, dims[i].h);
    }
    putchar('\n');
    return;
}

void print_tile_pos(sprite_t *sprite)
{
    tile_pos_t *tile_pos;
    size_t total_tile_pos;

    const char *trans_type;

    tile_pos = sprite_get_tile_pos(sprite, &total_tile_pos);

    puts("============= TILE POS ============");
    printf("%s%5s%5s%20s\n", "INDEX", "X", "Y", "TRANSFROM_TYPE");
    for(int i = 0; i < total_tile_pos; i++)
    {
        switch(tile_pos[i].transform)
        {
            case 0: trans_type = "TRANS_NONE"; break;
            case 1: trans_type = "TRANS_MIRROR"; break;
            case 2: trans_type = "TRANS_MIRROR_ROT180"; break;
            case 3: trans_type = "TRANS_ROT180"; break;
            default: trans_type = "UNKNOWN"; break;
        }
        printf("%5d%5d%5d%20s\n", tile_pos[i].tex_index, tile_pos[i].x, tile_pos[i].y, trans_type);
    }
    puts("===================================\n");
    return;
}

void print_pos_info(pos_info_t *pos_info, size_t count)
{
    printf("%s%10s\n", "COUNT", "OFFSET");
    for(int i = 0; i < count; i++)
    {
        printf("%5d%10d\n", pos_info[i].count, pos_info[i].offset);
    }
    return;
}

void save_textures(sprite_t *sprite, size_t texture_count, size_t palette_count)
{
    char buf[128];
    texture_t *texture;

    for(int i = 0; i < palette_count; i++)
    {
        for(int j = 0; j < texture_count; j++)
        {
            sprintf(buf, "tex_%02d_pal_%02d.png", j, i);
            texture = sprite_get_texture(sprite, j, i);
            if(texture)
            {
                int w = texture_get_width(texture);
                int h = texture_get_height(texture);
                image_save_png(buf, w, h, 4, texture_get_pixels(texture), w * 4);
                printf("[%s] saved\n", buf);
                texture_free(texture);
            }
        }
    }
}

int main(int argc, const char **argv)
{
    if(argc != 4) return 1;

    chunk_t *chunk = chunk_load(argv[1], atoi(argv[2]));
    sprite_t *sprite = sprite_load(chunk, atoi(argv[3]));
    if(!sprite)
    {
        fprintf(stderr, "Failed to load sprite\n");
        return 1;
    }

    printf("Textures in sprite: %lu\n", sprite_get_texture_count(sprite));
    printf("Palettes in sprite: %lu\n\n", sprite_get_palette_count(sprite));

    print_tex_info(sprite, sprite_get_texture_count(sprite));
    print_tile_pos(sprite);

    size_t count;
    pos_info_t *pos_info = sprite_get_tile_pos_info(sprite, &count);
    puts("= TILE POS INFO =");
    print_pos_info(pos_info, count);
    puts("=================\n");

    pos_info = sprite_get_animation_info(sprite, &count);
    puts("== ANIMATION ==");
    print_pos_info(pos_info, count);
    puts("===============\n");

    save_textures(sprite, sprite_get_texture_count(sprite), sprite_get_palette_count(sprite));

    sprite_free(sprite);
    return 0;
}
#include <errno.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "image_save.h"

enum
{
    TRANS_NONE = 0,
    TRANS_MIRROR,
    TRANS_MIRROR_ROT180,
    TRANS_ROT180
};

typedef struct
{
    int width;
    int height;
    SDL_Window *window;
    SDL_Renderer *render;

    SDL_Texture *tex;
} graphic_t;

typedef struct
{
    int width;
    int height;
    SDL_Texture *texture;
} tex_t;

static graphic_t *_this = NULL;

int graphic_init(const char *window_name, int width, int height, int render_target)
{
    graphic_t *res;
    SDL_Window *win = NULL;
    SDL_Renderer *render = NULL;

    SDL_Texture *tex = NULL;

    if(_this) return 1;

    res = (graphic_t*)malloc(sizeof(graphic_t));
    if(!res)
    {
        fprintf(stderr, "Failed to alloc memory!\n");
        return -1;
    }
    res->width = width;
    res->height = height;

    if(SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
            "Couldn't initialize SDL: %s", SDL_GetError());
        goto fail;
    }

    res->window = win = SDL_CreateWindow(
        window_name, 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI);
    if(!win)
    {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
            "CreateWindow failed: %s", SDL_GetError());
        goto fail;
    }

    res->render = render = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if(!render)
    {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
            "CreateRenderer failed: %s", SDL_GetError());
        goto fail;
    }

    if(render_target)
    {
        // For save image
        res->tex = tex = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, width, height);
        if(!tex)
        {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                "CreateTexture failed: %s", SDL_GetError());
            goto fail;
        }
        // TODO: Add failure check
        SDL_SetRenderTarget(render, tex);
        SDL_SetRenderDrawColor(render, 0, 0, 0, 0);
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    }

    _this = res;
    return 0;

fail:
    if(tex) SDL_DestroyTexture(tex);
    if(render) SDL_DestroyRenderer(render);
    if(win) SDL_DestroyWindow(win);
    if(res) free(res);
    SDL_Quit();
    return -1;
}

int graphic_get_width()
{
    if(!_this) return -1;

    return _this->width;
}

int graphic_get_height()
{
    if(!_this) return -1;

    return _this->height;
}

void graphic_show_window()
{
    if(!_this) return;

    SDL_ShowWindow(_this->window);
    return;
}

void graphic_draw_region(tex_t *texture, int x, int y, int transform)
{
    double rotate = 0.0f;
    SDL_RendererFlip filp = SDL_FLIP_NONE;

    if(!_this || !texture) return;

    SDL_Rect dstrect = { x, y, texture->width, texture->height };

    switch(transform & 0x3)
    {
        case TRANS_MIRROR_ROT180:       // 2
            rotate = 180.0f;
        case TRANS_MIRROR:              // 1
            filp = SDL_FLIP_HORIZONTAL;
            break;
        case TRANS_ROT180:              // 3
            rotate = 180.0f;
        case TRANS_NONE:                // 0
            break;
    }
    SDL_RenderCopyEx(_this->render, texture->texture, NULL, &dstrect, rotate, NULL, filp);
    return;
}

void graphic_clear()
{
    if(!_this) return;

    SDL_RenderClear(_this->render);
    return;
}

void graphic_present()
{
    if(!_this) return;

    SDL_RenderPresent(_this->render);
    return;
}

void graphic_quit()
{
    if(!_this) return;

    SDL_DestroyRenderer(_this->render);
    SDL_DestroyWindow(_this->window);

    if(_this->tex)
    {
        SDL_DestroyTexture(_this->tex);
    }

    free(_this);
    SDL_Quit();

    _this = NULL;

    return;
}

int graphic_get_texture_width(tex_t *tex)
{
    if(!tex) return -1;

    return tex->width;
}

int graphic_get_texture_height(tex_t *tex)
{
    if(!tex) return -1;

    return tex->height;
}

tex_t *graphic_create_texture(const void *pixels, int w, int h, int pitch)
{
    tex_t *res;
    SDL_Texture *tex = NULL;

    if(!_this) return NULL;

    res = (tex_t*)malloc(sizeof(tex_t));
    if(!res) return NULL;
    res->width = w;
    res->height = h;

    tex = SDL_CreateTexture(_this->render, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, w, h);
    if(!tex) goto fail;
    res->texture = tex;

    if(SDL_UpdateTexture(tex, NULL, pixels, pitch))
    {
        goto fail;
    }
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    return res;
fail:
    if(tex) SDL_DestroyTexture(tex);
    if(res) free(res);
    return NULL;
}

void graphic_destroy_texture(tex_t *tex)
{
    if(!tex) return;

    SDL_DestroyTexture(tex->texture);
    free(tex);
    return;
}

static int graphic_take_screenshot_impl(const char *filename, SDL_Rect *rect)
{
    int w, h;
    int res = -1;

    w = graphic_get_width();
    h = graphic_get_height();
    if(w <= 0 || h <= 0)
    {
        SDL_SetError("Invalid graphic!");
        goto fail1;
    }

    if(rect)
    {
        w = rect->w;
        h = rect->h;
    }

    void *pixel_data = malloc(w * h * 4);
    if(!pixel_data) return -1;

    if(SDL_RenderReadPixels(
        _this->render, rect,
        SDL_PIXELFORMAT_RGBA32,
        pixel_data, w * 4))
    {
        SDL_SetError("Failed to read render pixels");
        goto fail2;
    }

    if(!image_save_png(filename, w, h, 4, pixel_data, w * 4))
    {
        res = 0;
    }
    else
    {
        SDL_SetError("%s: %s", filename, strerror(errno));
    }

fail2:
    free(pixel_data);
fail1:
    return res;
}

int graphic_take_screenshot(const char *filename)
{
    return graphic_take_screenshot_impl(filename, NULL);
}

int graphic_take_region_screenshot(const char *filename, int x, int y, int w, int h)
{
    SDL_Rect rect = { x, y, w, h };

    if(x < 0 || y < 0 || w <= 0 || h <= 0)
    {
        return SDL_SetError("%s: Invalid param", __func__);
    }

    return graphic_take_screenshot_impl(filename, &rect);
}

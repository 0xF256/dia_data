/* MIT License
 * 
 * Copyright (c) 2025 SmithGoll
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "graphic.h"
#include <errno.h>
#include <string.h>

enum {
    TRANS_NONE = 0,
    TRANS_MIRROR,
    TRANS_MIRROR_ROT180,
    TRANS_ROT180
};

struct graphic_s {
    int width;
    int height;
    SDL_Window* window;
    SDL_Renderer* render;
};

struct tex_s {
    int width;
    int height;
    SDL_Texture* texture;
};

static graphic_t* _this = NULL;

int graphic_init(const char* window_name, int width, int height)
{
    graphic_t* res;
    SDL_Window* win = NULL;
    SDL_Renderer* render = NULL;

    if (_this)
        return 1;

    res = (graphic_t*)malloc(sizeof(graphic_t));
    if (!res) {
        fprintf(stderr, "Failed to alloc memory!\n");
        return -1;
    }
    res->width = width;
    res->height = height;

    if (SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
            "Couldn't initialize SDL: %s", SDL_GetError());
        goto fail;
    }

    res->window = win = SDL_CreateWindow(
        window_name,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_HIDDEN);
    if (!win) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
            "CreateWindow failed: %s", SDL_GetError());
        goto fail;
    }

    res->render = render = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!render) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
            "CreateRenderer failed: %s", SDL_GetError());
        goto fail;
    }

    SDL_RenderSetScale(render, 5.0f, 5.0f);

    _this = res;
    return 0;

fail:
    if (render) SDL_DestroyRenderer(render);
    if (win) SDL_DestroyWindow(win);
    if (res) free(res);
    SDL_Quit();
    return -1;
}

int graphic_get_width()
{
    if (!_this)
        return -1;

    return _this->width;
}

int graphic_get_height()
{
    if (!_this)
        return -1;

    return _this->height;
}

void graphic_show_window()
{
    if (!_this)
        return;

    SDL_ShowWindow(_this->window);
    return;
}

void graphic_draw_region(tex_t* texture, int x, int y, int transform)
{
    double rotate = 0.0f;
    SDL_RendererFlip filp = SDL_FLIP_NONE;

    if (!_this || !texture)
        return;

    SDL_Rect dstrect = { x, y, texture->width, texture->height };

    switch (transform & 0x3) {
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
    if (!_this)
        return;

    SDL_RenderClear(_this->render);
    return;
}

void graphic_present()
{
    if (!_this)
        return;

    SDL_RenderPresent(_this->render);
    return;
}

void graphic_quit()
{
    if (!_this)
        return;

    SDL_DestroyRenderer(_this->render);
    SDL_DestroyWindow(_this->window);

    free(_this);
    SDL_Quit();

    _this = NULL;
    return;
}

int graphic_get_texture_width(tex_t* tex)
{
    if (!tex)
        return -1;

    return tex->width;
}

int graphic_get_texture_height(tex_t* tex)
{
    if (!tex)
        return -1;

    return tex->height;
}

tex_t* graphic_create_texture(const void* pixels, int w, int h, int pitch)
{
    tex_t* res;
    SDL_Texture* tex = NULL;

    if (!_this)
        return NULL;

    res = (tex_t*)malloc(sizeof(tex_t));
    if (!res)
        return NULL;
    res->width = w;
    res->height = h;

    tex = SDL_CreateTexture(_this->render, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, w, h);
    if (!tex)
        goto fail;
    res->texture = tex;

    if (SDL_UpdateTexture(tex, NULL, pixels, pitch)) {
        goto fail;
    }
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    return res;
fail:
    if (tex) SDL_DestroyTexture(tex);
    if (res) free(res);
    return NULL;
}

void graphic_destroy_texture(tex_t* tex)
{
    if (!tex)
        return;

    SDL_DestroyTexture(tex->texture);
    free(tex);
    return;
}

/*
 * MIT License
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

#include "engine_tex_impl.h"

#include <SDL2/SDL.h>

#define FLIP_NONE 0
#define FLIP_X 1
#define FLIP_Y 2
#define FLIP_XY (FLIP_X|FLIP_Y)

void* module_new(void* pixels, int w, int h, void* user_data)
{
    SDL_Renderer* render;
    SDL_Texture* texture;

    render = (SDL_Renderer*)user_data;
    texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STATIC, w, h);
    if (!texture) return NULL;

    if (SDL_UpdateTexture(texture, NULL, pixels, w *  4)) {
        SDL_DestroyTexture(texture);
        return NULL;
    }
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    return (void*)texture;
}

void module_free(void* module, void* user_data)
{
    SDL_Texture* texture;

    texture = (SDL_Texture*)module;
    if (texture)
        SDL_DestroyTexture(texture);
    return;
}

void module_paint(void* module, int x, int y, int flip, void* user_data)
{
    int w, h;
    double rotate;
    SDL_Rect real_rect;
    SDL_Texture* texture;
    SDL_Renderer* render;
    SDL_RendererFlip real_flip;

    render = (SDL_Renderer*)user_data;
    texture = (SDL_Texture*)module;
    if (!texture || !render)
        return;

    if (SDL_QueryTexture(texture, NULL, NULL, &w, &h))
        return;

    rotate = 0;
    real_flip = SDL_FLIP_NONE;
    real_rect.x = x;
    real_rect.y = y;
    real_rect.w = w;
    real_rect.h = h;
    switch (flip & FLIP_XY) {
    case FLIP_Y:
        rotate = 180;
    case FLIP_X:
        real_flip = SDL_FLIP_HORIZONTAL;
        break;
    case FLIP_XY:
        rotate = 180;
        break;
    }

    SDL_RenderCopyEx(render, texture, NULL, &real_rect, rotate, NULL, real_flip);
    return;
}

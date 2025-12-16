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

#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

int graphic_width = 0;
int graphic_height = 0;
SDL_Window* graphic_window = NULL;
SDL_Renderer* graphic_render = NULL;

void graphic_quit();

int graphic_init(const char* window_name, int width, int height)
{
    if (graphic_window)
        return 1;

    if (SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
            "Couldn't initialize SDL: %s", SDL_GetError());
        goto fail;
    }

    graphic_window = SDL_CreateWindow(
        window_name,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_HIDDEN);
    if (!graphic_window) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
            "CreateWindow failed: %s", SDL_GetError());
        goto fail;
    }

    graphic_render = SDL_CreateRenderer(graphic_window, -1, SDL_RENDERER_ACCELERATED);
    if (!graphic_render) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
            "CreateRenderer failed: %s", SDL_GetError());
        goto fail;
    }

    SDL_RenderSetScale(graphic_render, 5.0f, 5.0f);
    return 0;

fail:
    graphic_quit();
    return -1;
}

int graphic_get_width()
{
    if (!graphic_window)
        return -1;

    return graphic_width;
}

int graphic_get_height()
{
    if (!graphic_window)
        return -1;

    return graphic_height;
}

void graphic_show_window()
{
    if (!graphic_window)
        return;

    SDL_ShowWindow(graphic_window);
    return;
}

void graphic_clear()
{
    if (!graphic_render)
        return;

    SDL_RenderClear(graphic_render);
    return;
}

void graphic_present()
{
    if (!graphic_render)
        return;

    SDL_RenderPresent(graphic_render);
    return;
}

void graphic_quit()
{
    if (graphic_render) {
        SDL_DestroyRenderer(graphic_render);
        graphic_render = NULL;
    }

    if (graphic_window) {
        SDL_DestroyWindow(graphic_window);
        graphic_window = NULL;
    }

    SDL_Quit();
    return;
}

#ifdef __cplusplus
}
#endif

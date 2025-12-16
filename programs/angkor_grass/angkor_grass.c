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

#include "chunk.h"
#include "graphic.h"
#include "sprite.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#define FPS 8

sprite_t* grass = NULL;
sprite_t* bg_spr = NULL;

int anim_flip = 0;
int frames_count = 0;

SDL_bool running = SDL_TRUE;

void draw_background()
{
    graphic_clear();
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            sprite_draw_module(bg_spr, 0, x * 24, y * 24, 0);
        }
    }
    return;
}

void draw_animation()
{
    if (anim_flip > FLIP_XY)
        sprite_draw_aframe(grass, frames_count, 24, 24, anim_flip - FLIP_XY - 1);
    else
        sprite_draw_aframe_abs(grass, frames_count, 24, 24, anim_flip);
    frames_count = (frames_count + 1) % 8;
    return;
}

int texture_init()
{
    sprite_t* spr;
    chunk_t* chunk;
    file_handle_t* handle;

    chunk = chunk_open("0.f");
    if (!chunk) {
        fprintf(stderr, "Failed to open chunk file\n");
        return 1;
    }

    int idx[] = { 1, chunk_get_data_count(chunk) - 1 };
    for (int i = 0; i < 2; i++) {
        handle = chunk_get_data(chunk, idx[i]);
        if (!handle) {
            fprintf(stderr, "Failed to get data\n");
            chunk_free(chunk);
            return 1;
        }

        spr = sprite_load(handle, graphic_render);
        if (!spr) {
            fprintf(stderr, "Failed to load sprite\n");
            chunk_free(chunk);
            file_close(handle);
            return 1;
        }

        if (i)
            bg_spr = spr;
        else
            grass = spr;

        file_close(handle);
    }
    chunk_free(chunk);

    return 0;
}

void main_loop()
{
    SDL_Event event;

#ifndef __EMSCRIPTEN__
    uint32_t begin = SDL_GetTicks();
#endif

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            running = SDL_FALSE;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
                anim_flip = (anim_flip + 1) % ((FLIP_XY + 1) * 2);
            break;
        }
    }

#ifdef __EMSCRIPTEN__
    if (!running) {
        emscripten_cancel_main_loop();
    }
#else
    if (!running) {
        return;
    }
#endif

    draw_background();
    draw_animation();

    graphic_present();

#ifndef __EMSCRIPTEN__
    uint32_t current = SDL_GetTicks();
    uint32_t cost = current - begin;
    long delay = (1000 / FPS) - cost;
    if (delay > 0) {
        SDL_Delay(delay);
    }
#endif
}

int main()
{
    if (graphic_init("Angkor Grass Animation", 120 * 3, 120 * 3)) {
        fprintf(stderr, "Failed to init graphic\n");
        return -1;
    }

    if (texture_init()) {
        fprintf(stderr, "Failed to load texture\n");
        return -1;
    }

    graphic_show_window();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, FPS, 1);
#else
    while (running) {
        main_loop();
    }
#endif

release_res:
    sprite_free(bg_spr);
    sprite_free(grass);
    graphic_quit();
    return 0;
}

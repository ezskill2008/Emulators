#include "graphics.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define PIXEL_ON  0xFFFFFFFFu  /* ARGB white */
#define PIXEL_OFF 0xFF000000u  /* ARGB black */

struct graphics {
    SDL_Window*     window;
    SDL_Renderer*   renderer;
    SDL_Texture*    texture;
    int             open;
    uint32_t        buffer[CHIP8_WIDTH * CHIP8_HEIGHT];
};

graphics*
graphics_create(const char* title, int scale) {
    if (scale < 1) scale = 1;

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_InitSubSystem failed: %s\n", SDL_GetError());
        return NULL;
    }

    graphics* g = calloc(1, sizeof * g);
    if (!g) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return NULL;
    }

    g->window = SDL_CreateWindow(title,
        CHIP8_WIDTH * scale, CHIP8_HEIGHT * scale,
        SDL_WINDOW_RESIZABLE);
    if (!g->window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        goto fail;
    }

    g->renderer = SDL_CreateRenderer(g->window, NULL);  /* NULL = best available */
    if (!g->renderer) {
        g->renderer = SDL_CreateRenderer(g->window, "software");
    }
    if (!g->renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        goto fail;
    }

    /* Keeps the 64:32 aspect ratio (letterboxed) when the window is resized. */
    if (!SDL_SetRenderLogicalPresentation(g->renderer,
        CHIP8_WIDTH, CHIP8_HEIGHT,
        SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        fprintf(stderr, "SDL_SetRenderLogicalPresentation failed: %s\n", SDL_GetError());
        goto fail;
    }

    g->texture = SDL_CreateTexture(g->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        CHIP8_WIDTH, CHIP8_HEIGHT);
    if (!g->texture) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        goto fail;
    }

    /* Nearest-neighbour scaling keeps pixels sharp. */
    SDL_SetTextureScaleMode(g->texture, SDL_SCALEMODE_NEAREST);

    g->open = 1;

    return g;

fail:
    graphics_destroy(g);
    return NULL;
}

void
graphics_draw(graphics* g, const uint32_t pixels[CHIP8_WIDTH * CHIP8_HEIGHT]) {
    if (!g || !pixels) return;

    for (int i = 0; i < CHIP8_WIDTH * CHIP8_HEIGHT; i++) {
        g->buffer[i] = pixels[i] ? PIXEL_ON : PIXEL_OFF;
    }

    SDL_UpdateTexture(g->texture, NULL, g->buffer, CHIP8_WIDTH * (int)sizeof(uint32_t));
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255);
    SDL_RenderClear(g->renderer);
    SDL_RenderTexture(g->renderer, g->texture, NULL, NULL);
    SDL_RenderPresent(g->renderer);
}

void
graphics_destroy(graphics* g) {
    if (!g) return;
    if (g->texture)  SDL_DestroyTexture(g->texture);
    if (g->renderer) SDL_DestroyRenderer(g->renderer);
    if (g->window)   SDL_DestroyWindow(g->window);
    free(g);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

int
graphics_window_open(graphics* g) {
    if (!g || !g->open) return 0;

    SDL_PumpEvents();
    if (SDL_HasEvent(SDL_EVENT_QUIT) ||
        SDL_HasEvent(SDL_EVENT_WINDOW_CLOSE_REQUESTED)) {
        g->open = 0;
    }
    return g->open;
}

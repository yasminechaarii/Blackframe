#include "settings.h"

SDL_Window   *window   = NULL;
SDL_Renderer *renderer = NULL;
int running = 1;

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return 1;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))       return 1;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return 1;

    window = SDL_CreateWindow("Menu Settings",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return 1;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return 1;

    loadAssets(renderer);

    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            handleUI(&e, window);
        }
        SDL_RenderClear(renderer);
        renderUI(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    cleanupAssets();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
    return 0;
}

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "save2.h"
#include "load2.h"

#define WIDTH 1280
#define HEIGHT 720

typedef enum {
    MAIN_MENU,
    SAVE_MENU,
    LOAD_MENU
} AppState;

int button_click(Button *b, SDL_Event *e) {
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    return (e->type == SDL_MOUSEBUTTONDOWN &&
            mx >= b->rect.x &&
            mx <= b->rect.x + b->rect.w &&
            my >= b->rect.y &&
            my <= b->rect.y + b->rect.h);
}

int main() {

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("Menu",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, 0);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture *background = IMG_LoadTexture(renderer, "assets/background.png");

    AppState state = MAIN_MENU;
    int selected_button = 0;

    SDL_Event e;
    int running = 1;

    while (running) {

        while (SDL_PollEvent(&e)) {

            if (e.type == SDL_QUIT)
                running = 0;

            /* MAIN MENU: click anywhere */
            if (state == MAIN_MENU) {
                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    state = SAVE_MENU;
                }
            }

            else if (state == SAVE_MENU) {
                Button saveBtn = get_save_button(WIDTH, HEIGHT);
                Button noSaveBtn = get_nosave_button(WIDTH, HEIGHT);

                if (button_click(&saveBtn, &e)) {
                    selected_button = 3;
                    state = LOAD_MENU;
                }

                if (button_click(&noSaveBtn, &e)) {
                    selected_button = 2;
                }
            }

            else if (state == LOAD_MENU) {
                Button loadingBtn = get_loading_button(WIDTH, HEIGHT);
                Button newBtn = get_new_button(WIDTH, HEIGHT);

                if (button_click(&loadingBtn, &e))
                    selected_button = 3;

                if (button_click(&newBtn, &e))
                    selected_button = 4;

                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                    state = MAIN_MENU;
                    selected_button = 0;
                }
            }
        }

        SDL_RenderClear(renderer);

        if (state == MAIN_MENU) {
            SDL_RenderCopy(renderer, background, NULL, NULL);
        }

        else if (state == SAVE_MENU) {
            SDL_RenderCopy(renderer, background, NULL, NULL);
            save2(renderer, WIDTH, HEIGHT, selected_button);
        }

        else if (state == LOAD_MENU) {
            load2(renderer, WIDTH, HEIGHT, selected_button);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(background);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}

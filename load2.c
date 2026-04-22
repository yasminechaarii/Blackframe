#include "load2.h"

#define BTN_WIDTH 400
#define BTN_HEIGHT 80

Button get_loading_button(int width, int height) {
    Button b = {
        width/2 - BTN_WIDTH/2,
        height/2 - 100,
        BTN_WIDTH,
        BTN_HEIGHT
    };
    return b;
}

Button get_new_button(int width, int height) {
    Button b = {
        width/2 - BTN_WIDTH/2,
        height/2 + 20,
        BTN_WIDTH,
        BTN_HEIGHT
    };
    return b;
}

void load2(SDL_Renderer *r, int width, int height, int selected_button) {
    Button loadingBtn = get_loading_button(width, height);
    Button newBtn = get_new_button(width, height);

    SDL_Texture *bg = IMG_LoadTexture(r, "assets/background3.png");
    SDL_Texture *loading = IMG_LoadTexture(r, "assets/loading.png");
    SDL_Texture *newb = IMG_LoadTexture(r, "assets/new.png");

    SDL_RenderCopy(r, bg, NULL, NULL);
    SDL_RenderCopy(r, loading, NULL, &loadingBtn.rect);
    SDL_RenderCopy(r, newb, NULL, &newBtn.rect);

    if (mouse_on_button(loadingBtn) || selected_button == 3)
        draw_black_box(r, loadingBtn.rect);

    if (mouse_on_button(newBtn) || selected_button == 4)
        draw_black_box(r, newBtn.rect);

    SDL_DestroyTexture(bg);
    SDL_DestroyTexture(loading);
    SDL_DestroyTexture(newb);
}

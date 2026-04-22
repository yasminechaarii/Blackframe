#include "save2.h"

Button get_save_button(int width, int height) {
    Button b = {{width/2 - 330, height/2 + 35, 260, 90}};
    return b;
}

Button get_nosave_button(int width, int height) {
    Button b = {{width/2 + 40, height/2 + 35, 260, 90}};
    return b;
}

int mouse_on_button(Button b) {
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    return (mx >= b.rect.x &&
            mx <= b.rect.x + b.rect.w &&
            my >= b.rect.y &&
            my <= b.rect.y + b.rect.h);
}

void draw_black_box(SDL_Renderer *r, SDL_Rect rect) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);

    SDL_RenderDrawRect(r, &rect);

    SDL_Rect r2 = {rect.x - 2, rect.y - 2, rect.w + 4, rect.h + 4};
    SDL_RenderDrawRect(r, &r2);
}

void save2(SDL_Renderer *r, int width, int height, int selected_button) {

    Button save = get_save_button(width, height);
    Button nosave = get_nosave_button(width, height);

    SDL_Texture *saveTexture = IMG_LoadTexture(r, "assets/save.png");
    SDL_Texture *noSaveTexture = IMG_LoadTexture(r, "assets/dont_save.png");

    TTF_Font *font1 = TTF_OpenFont("assets/font.ttf", 70);
    TTF_Font *font2 = TTF_OpenFont("assets/font.ttf", 92);

    SDL_Color beige = {230, 215, 180, 255};
    SDL_Color red   = {220, 50, 50, 255};

    SDL_Surface *line1Surface = TTF_RenderText_Blended(font1, "DO YOU WANNA", beige);
    SDL_Surface *line2aSurface = TTF_RenderText_Blended(font2, "SAVE", red);
    SDL_Surface *line2bSurface = TTF_RenderText_Blended(font2, " THE GAME?", beige);

    SDL_Texture *line1Texture = SDL_CreateTextureFromSurface(r, line1Surface);
    SDL_Texture *line2aTexture = SDL_CreateTextureFromSurface(r, line2aSurface);
    SDL_Texture *line2bTexture = SDL_CreateTextureFromSurface(r, line2bSurface);

    SDL_Rect line1Rect = {
        width/2 - line1Surface->w/2,
        90,
        line1Surface->w,
        line1Surface->h
    };

    int totalLine2Width = line2aSurface->w + line2bSurface->w;

    SDL_Rect line2aRect = {
        width/2 - totalLine2Width/2,
        180,
        line2aSurface->w,
        line2aSurface->h
    };

    SDL_Rect line2bRect = {
        line2aRect.x + line2aSurface->w,
        180,
        line2bSurface->w,
        line2bSurface->h
    };

    SDL_RenderCopy(r, saveTexture, NULL, &save.rect);
    SDL_RenderCopy(r, noSaveTexture, NULL, &nosave.rect);

    SDL_RenderCopy(r, line1Texture, NULL, &line1Rect);
    SDL_RenderCopy(r, line2aTexture, NULL, &line2aRect);
    SDL_RenderCopy(r, line2bTexture, NULL, &line2bRect);

    if (mouse_on_button(save) || selected_button == 1)
        draw_black_box(r, save.rect);

    if (mouse_on_button(nosave) || selected_button == 2)
        draw_black_box(r, nosave.rect);

    SDL_FreeSurface(line1Surface);
    SDL_FreeSurface(line2aSurface);
    SDL_FreeSurface(line2bSurface);

    SDL_DestroyTexture(line1Texture);
    SDL_DestroyTexture(line2aTexture);
    SDL_DestroyTexture(line2bTexture);

    SDL_DestroyTexture(saveTexture);
    SDL_DestroyTexture(noSaveTexture);

    TTF_CloseFont(font1);
    TTF_CloseFont(font2);
}

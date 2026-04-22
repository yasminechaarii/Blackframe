#ifndef BUTTON_H
#define BUTTON_H

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct {
    SDL_Rect rect;
    SDL_Texture *normal;
    SDL_Texture *hover;
    bool isHovered;
} Button;

void initButton(Button *b, int x, int y, int w, int h,
                SDL_Texture *normal,
                SDL_Texture *hover);

void updateButtonHover(Button *b, int mouseX, int mouseY);

void renderButton(Button *b, SDL_Renderer *renderer);

#endif

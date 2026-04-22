#ifndef SAVE2_H
#define SAVE2_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

typedef struct {
    SDL_Rect rect;
} Button;

Button get_save_button(int width, int height);
Button get_nosave_button(int width, int height);
int mouse_on_button(Button b);
void draw_black_box(SDL_Renderer *r, SDL_Rect rect);
void save2(SDL_Renderer *r, int width, int height, int selected_button);

#endif

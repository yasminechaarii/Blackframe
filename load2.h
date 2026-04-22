#ifndef LOAD2_H
#define LOAD2_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "save2.h"

Button get_loading_button(int width, int height);
Button get_new_button(int width, int height);
void load2(SDL_Renderer *r, int width, int height, int selected_button);

#endif

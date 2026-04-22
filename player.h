#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include "background.h"

typedef struct Player {
    SDL_FRect rect;
    float vx;
    float vy;
    int on_ground;
} Player;

void player_init(Player* p, float x, float y);
void player_handle_input(Player* p, const Uint8* keys);
void player_update(Player* p, const Level* level, float dt);
void player_draw(SDL_Renderer* renderer, const Player* p, const Camera* cam);

#endif


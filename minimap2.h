#ifndef MINIMAP2_H
#define MINIMAP2_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define NB_RED 8
#define MAX_LIVES 3
#define MOVE_HORIZONTAL 0
#define MOVE_VERTICAL 1

typedef struct
{
    SDL_Texture *bg;
    SDL_Rect mapRect;
    SDL_Rect dotRect;
    int worldW;
    int worldH;
} minimap;

typedef struct
{
    SDL_Rect rect;
    int minPos;
    int maxPos;
    int dir;
    int speed;
    int moveMode;
    int dx;
    int dy;
    int active;
    int vanish;
} RedBlock;

int initMinimap(minimap *m, SDL_Renderer *r, const char *path,
                int x, int y, int w, int h, int worldW, int worldH);

void updateMinimap(minimap *m, SDL_Rect player);

int solidAt(SDL_Surface *mask, int x, int y);

void updateBlocks(RedBlock red[]);

void drawBlocksLivesAndMinimap(SDL_Renderer *renderer, SDL_Texture *redTexture,
                               RedBlock red[], minimap *mini, SDL_Rect player,
                               SDL_Rect camera, int lives, int screenW, int screenH);

#endif

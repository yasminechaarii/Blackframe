#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "minimap2.h"

#define PLAYER_W 12
#define PLAYER_H 22
#define MOVE_SPEED 4
#define GRAVITY 1
#define JUMP_SPEED -14
#define MAX_FALL_SPEED 10
#define CAMERA_WORLD_W 420

int main(void)
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Event e;
    SDL_Surface *mask = NULL;
    SDL_Surface *bgSurface = NULL;
    SDL_Surface *redSurface = NULL;
    SDL_Texture *background = NULL;
    SDL_Texture *redTexture = NULL;
    SDL_Rect player, camera;
    minimap mini;
    RedBlock red[NB_RED];
    int screenW = 1280, screenH = 720;
    int worldW, worldH;
    int running = 1, onGround = 1, velY = 0, lives = MAX_LIVES;
    int spawnX = 50, spawnY = 0, oldX, left, right, top, bottom, i;
    const Uint8 *keys;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) return 1;

    window = SDL_CreateWindow("Spy Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenW, screenH, 0);
    if (!window) return 1;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return 1;

    bgSurface = IMG_Load("assets/level1.without.jpeg");
    if (!bgSurface) return 1;
    background = SDL_CreateTextureFromSurface(renderer, bgSurface);
    if (!background) return 1;

    mask = IMG_Load("assets/level1_mask.png");
    if (!mask) return 1;

    redSurface = IMG_Load("assets/red_platform.jpg");
    if (redSurface) redTexture = SDL_CreateTextureFromSurface(renderer, redSurface);

    worldW = mask->w;
    worldH = mask->h;

    player.w = PLAYER_W;
    player.h = PLAYER_H;
    player.x = spawnX;
    player.y = 0;
    for (i = 0; i < mask->h - PLAYER_H - 2; i++)
    {
        if (solidAt(mask, spawnX + PLAYER_W / 2, i + PLAYER_H) &&
            !solidAt(mask, spawnX + 1, i + 1) &&
            !solidAt(mask, spawnX + PLAYER_W - 1, i + 1) &&
            !solidAt(mask, spawnX + 1, i + PLAYER_H - 1) &&
            !solidAt(mask, spawnX + PLAYER_W - 1, i + PLAYER_H - 1))
        {
            player.y = i > 0 ? i - 1 : 0;
            break;
        }
    }
    spawnY = player.y;

    camera.w = CAMERA_WORLD_W;
    camera.h = (CAMERA_WORLD_W * screenH) / screenW;
    if (camera.h > worldH) camera.h = worldH;
    if (camera.h < 180) camera.h = 180;
    camera.x = 0;
    camera.y = 0;

    red[0] = (RedBlock){{140,  player.y - 35, 70, 12}, 140, 220, 1, 1, MOVE_HORIZONTAL, 0, 0, 1, 0};
    red[1] = (RedBlock){{260,  player.y - 60, 70, 12}, player.y - 90,  player.y - 35, 1, 1, MOVE_VERTICAL,   0, 0, 1, 0};
    red[2] = (RedBlock){{430,  player.y - 40, 70, 12}, 430, 510, 1, 1, MOVE_HORIZONTAL, 0, 0, 1, 1};
    red[3] = (RedBlock){{620,  player.y - 75, 70, 12}, player.y - 100, player.y - 45, 1, 1, MOVE_VERTICAL,   0, 0, 1, 0};
    red[4] = (RedBlock){{860,  player.y - 45, 70, 12}, 860, 940, 1, 1, MOVE_HORIZONTAL, 0, 0, 1, 0};
    red[5] = (RedBlock){{1080, player.y - 75, 70, 12}, player.y - 105, player.y - 45, 1, 1, MOVE_VERTICAL,   0, 0, 1, 0};
    red[6] = (RedBlock){{1320, player.y - 45, 70, 12}, 1320, 1400, 1, 1, MOVE_HORIZONTAL, 0, 0, 1, 0};
    red[7] = (RedBlock){{1540, player.y - 75, 75, 12}, player.y - 105, player.y - 45, 1, 1, MOVE_VERTICAL,   0, 0, 1, 0};

    if (!initMinimap(&mini, renderer, "assets/minimap.level1.jpeg", screenW - 300, 20, 280, 140, worldW, worldH))
        if (!initMinimap(&mini, renderer, "assets/minimap.level1.jpg", screenW - 300, 20, 280, 140, worldW, worldH))
            return 1;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = 0;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE && onGround)
            {
                velY = JUMP_SPEED;
                onGround = 0;
            }
        }

        keys = SDL_GetKeyboardState(NULL);
        oldX = player.x;

        if (keys[SDL_SCANCODE_LEFT]) player.x -= MOVE_SPEED;
        if (keys[SDL_SCANCODE_RIGHT]) player.x += MOVE_SPEED;

        if (player.x < 0) player.x = 0;
        if (player.x + player.w > worldW) player.x = worldW - player.w;

        if (solidAt(mask, player.x, player.y + 2) ||
            solidAt(mask, player.x, player.y + player.h - 2) ||
            solidAt(mask, player.x + player.w - 1, player.y + 2) ||
            solidAt(mask, player.x + player.w - 1, player.y + player.h - 2))
            player.x = oldX;

        updateBlocks(red);

        velY += GRAVITY;
        if (velY > MAX_FALL_SPEED) velY = MAX_FALL_SPEED;
        player.y += velY;
        onGround = 0;

        left = player.x + 1;
        right = player.x + player.w - 1;
        top = player.y;
        bottom = player.y + player.h;

        if (velY >= 0 && (solidAt(mask, left, bottom) || solidAt(mask, right, bottom)))
        {
            while (solidAt(mask, left, player.y + player.h) || solidAt(mask, right, player.y + player.h)) player.y--;
            velY = 0;
            onGround = 1;
        }

        if (velY < 0 && (solidAt(mask, left, top) || solidAt(mask, right, top)))
        {
            while (solidAt(mask, left, player.y) || solidAt(mask, right, player.y)) player.y++;
            velY = 0;
        }

        for (i = 0; i < NB_RED; i++)
        {
            SDL_Rect test = player;
            if (!red[i].active) continue;
            test.y += 2;

            if (SDL_HasIntersection(&test, &red[i].rect) &&
                velY >= 0 &&
                player.y + player.h <= red[i].rect.y + 14)
            {
                if (red[i].vanish) red[i].active = 0;
                else
                {
                    player.y = red[i].rect.y - player.h;
                    velY = 0;
                    onGround = 1;
                    player.x += red[i].dx;
                    player.y += red[i].dy;
                }
            }
        }

        if (player.y + player.h >= worldH - 5)
        {
            lives--;
            if (lives <= 0) running = 0;
            else
            {
                player.x = spawnX;
                player.y = spawnY;
                velY = 0;
                onGround = 1;
            }
        }

        camera.x = player.x + player.w / 2 - camera.w / 2;
        camera.y = player.y + player.h / 2 - camera.h / 2;
        if (camera.x < 0) camera.x = 0;
        if (camera.y < 0) camera.y = 0;
        if (camera.x > worldW - camera.w) camera.x = worldW - camera.w;
        if (camera.y > worldH - camera.h) camera.y = worldH - camera.h;

        updateMinimap(&mini, player);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        {
            SDL_Rect src = {camera.x, camera.y, camera.w, camera.h};
            SDL_Rect dst = {0, 0, screenW, screenH};
            SDL_RenderCopy(renderer, background, &src, &dst);
        }

        drawBlocksLivesAndMinimap(renderer, redTexture, red, &mini, player, camera, lives, screenW, screenH);
        SDL_RenderPresent(renderer);
    }

    if (mini.bg) SDL_DestroyTexture(mini.bg);
    if (redTexture) SDL_DestroyTexture(redTexture);
    if (redSurface) SDL_FreeSurface(redSurface);
    SDL_FreeSurface(mask);
    if (background) SDL_DestroyTexture(background);
    SDL_FreeSurface(bgSurface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

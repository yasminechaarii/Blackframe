#include "minimap2.h"
#include <stdio.h>

int initMinimap(minimap *m, SDL_Renderer *r, const char *path,
                int x, int y, int w, int h, int worldW, int worldH)
{
    SDL_Surface *s = IMG_Load(path);
    if (!s)
    {
        printf("IMG_Load minimap error: %s\n", IMG_GetError());
        return 0;
    }

    m->bg = SDL_CreateTextureFromSurface(r, s);
    SDL_FreeSurface(s);

    if (!m->bg)
    {
        printf("Texture minimap error: %s\n", SDL_GetError());
        return 0;
    }

    m->mapRect.x = x;
    m->mapRect.y = y;
    m->mapRect.w = w;
    m->mapRect.h = h;
    m->dotRect.w = 8;
    m->dotRect.h = 8;
    m->worldW = worldW;
    m->worldH = worldH;
    return 1;
}

void updateMinimap(minimap *m, SDL_Rect player)
{
    float sx = (float)m->mapRect.w / (float)m->worldW;
    float sy = (float)m->mapRect.h / (float)m->worldH;
    int cx = player.x + player.w / 2;
    int cy = player.y + player.h / 2;

    m->dotRect.x = m->mapRect.x + (int)(cx * sx) - m->dotRect.w / 2;
    m->dotRect.y = m->mapRect.y + (int)(cy * sy) - m->dotRect.h / 2;

    if (m->dotRect.x < m->mapRect.x) m->dotRect.x = m->mapRect.x;
    if (m->dotRect.y < m->mapRect.y) m->dotRect.y = m->mapRect.y;
    if (m->dotRect.x > m->mapRect.x + m->mapRect.w - m->dotRect.w)
        m->dotRect.x = m->mapRect.x + m->mapRect.w - m->dotRect.w;
    if (m->dotRect.y > m->mapRect.y + m->mapRect.h - m->dotRect.h)
        m->dotRect.y = m->mapRect.y + m->mapRect.h - m->dotRect.h;
}

int solidAt(SDL_Surface *mask, int x, int y)
{
    Uint8 *p;
    Uint32 pixel;
    Uint8 r, g, b;
    int bpp;

    if (x < 0 || y < 0 || x >= mask->w || y >= mask->h) return 1;

    bpp = mask->format->BytesPerPixel;
    p = (Uint8 *)mask->pixels + y * mask->pitch + x * bpp;

    if (bpp == 1) pixel = *p;
    else if (bpp == 2) pixel = *(Uint16 *)p;
    else if (bpp == 3)
    {
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            pixel = p[0] << 16 | p[1] << 8 | p[2];
        else
            pixel = p[0] | p[1] << 8 | p[2] << 16;
    }
    else pixel = *(Uint32 *)p;

    SDL_GetRGB(pixel, mask->format, &r, &g, &b);
    return (r < 40 && g < 40 && b < 40);
}

void updateBlocks(RedBlock red[])
{
    int i;

    for (i = 0; i < NB_RED; i++)
    {
        if (!red[i].active) continue;

        red[i].dx = 0;
        red[i].dy = 0;

        if (red[i].moveMode == MOVE_HORIZONTAL)
        {
            red[i].rect.x += red[i].speed * red[i].dir;
            red[i].dx = red[i].speed * red[i].dir;

            if (red[i].rect.x <= red[i].minPos)
            {
                red[i].rect.x = red[i].minPos;
                red[i].dir = 1;
            }

            if (red[i].rect.x >= red[i].maxPos)
            {
                red[i].rect.x = red[i].maxPos;
                red[i].dir = -1;
            }
        }
        else
        {
            red[i].rect.y += red[i].speed * red[i].dir;
            red[i].dy = red[i].speed * red[i].dir;

            if (red[i].rect.y <= red[i].minPos)
            {
                red[i].rect.y = red[i].minPos;
                red[i].dir = 1;
            }

            if (red[i].rect.y >= red[i].maxPos)
            {
                red[i].rect.y = red[i].maxPos;
                red[i].dir = -1;
            }
        }
    }
}

void drawBlocksLivesAndMinimap(SDL_Renderer *renderer, SDL_Texture *redTexture,
                               RedBlock red[], minimap *mini, SDL_Rect player,
                               SDL_Rect camera, int lives, int screenW, int screenH)
{
    int i;
    float scaleX = (float)screenW / (float)camera.w;
    float scaleY = (float)screenH / (float)camera.h;

    for (i = 0; i < NB_RED; i++)
    {
        SDL_Rect rr;

        if (!red[i].active) continue;

        rr.x = (int)((red[i].rect.x - camera.x) * scaleX);
        rr.y = (int)((red[i].rect.y - camera.y) * scaleY);
        rr.w = (int)(red[i].rect.w * scaleX);
        rr.h = (int)(red[i].rect.h * scaleY);

        if (redTexture) SDL_RenderCopy(renderer, redTexture, NULL, &rr);
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &rr);
        }
    }

    {
        SDL_Rect pr;
        pr.x = (int)((player.x - camera.x) * scaleX);
        pr.y = (int)((player.y - camera.y) * scaleY);
        pr.w = (int)(player.w * scaleX);
        pr.h = (int)(player.h * scaleY);

        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_RenderFillRect(renderer, &pr);
    }

    for (i = 0; i < MAX_LIVES; i++)
    {
        SDL_Rect life = {20 + i * 22, 20, 16, 16};

        if (i < lives) SDL_SetRenderDrawColor(renderer, 255, 230, 0, 255);
        else SDL_SetRenderDrawColor(renderer, 90, 90, 90, 255);

        SDL_RenderFillRect(renderer, &life);
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderDrawRect(renderer, &life);
    }

    SDL_RenderCopy(renderer, mini->bg, NULL, &mini->mapRect);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &mini->dotRect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &mini->mapRect);
}

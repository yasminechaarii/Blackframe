#include "enigme.h"
#include "button.h"
#include "quiz.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>

void runMenu(SDL_Renderer *renderer)
{
    SDL_Event e;
    int running = 1;

    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    // 🔊 SOUND
    Mix_Music *music = Mix_LoadMUS("assets/suspense.mp3");
    Mix_PlayMusic(music, -1);

    Mix_Chunk *click = Mix_LoadWAV("assets/click.wav");

    TTF_Font *fontTitle = TTF_OpenFont("assets/font.ttf", 70);
    TTF_Font *fontBtn = TTF_OpenFont("assets/font.ttf", 30);

    SDL_Color white = {255,255,255};
    SDL_Color black = {0,0,0};

    SDL_Texture *bg = IMG_LoadTexture(renderer,"assets/background.png");

    SDL_Texture *q = IMG_LoadTexture(renderer,"assets/btn_quiz.png");
    SDL_Texture *qh = IMG_LoadTexture(renderer,"assets/hover_quiz.png");

    SDL_Texture *p = IMG_LoadTexture(renderer,"assets/btn_puzzle.png");
    SDL_Texture *ph = IMG_LoadTexture(renderer,"assets/hover_puzzle.png");

    SDL_Texture *quit = IMG_LoadTexture(renderer,"assets/quitter.png");
    SDL_Texture *quith = IMG_LoadTexture(renderer,"assets/hover_quitter.png");

    int W=1000,H=700;

    Button b1,b2,b3;
    initButton(&b1, W/2-270, H/2, 250,120, q,qh);
    initButton(&b2, W/2+20, H/2, 250,120, p,ph);
    initButton(&b3, 30, H-100, 180,70, quit,quith);

    // TITLE
    SDL_Surface *ts = TTF_RenderText_Blended(fontTitle,"Jumanji Nexus",white);
    SDL_Texture *tt = SDL_CreateTextureFromSurface(renderer,ts);
    SDL_Rect tr = {W/2-ts->w/2,50,ts->w,ts->h};
    SDL_FreeSurface(ts);

    // TEXT BUTTONS
    SDL_Surface *qs = TTF_RenderText_Blended(fontBtn,"Quiz",black);
    SDL_Texture *qt = SDL_CreateTextureFromSurface(renderer,qs);
    SDL_Rect qr = {b1.rect.x + b1.rect.w/2 - qs->w/2,
                   b1.rect.y + b1.rect.h/2 - qs->h/2,
                   qs->w, qs->h};
    SDL_FreeSurface(qs);

    SDL_Surface *ps = TTF_RenderText_Blended(fontBtn,"Puzzle",black);
    SDL_Texture *pt = SDL_CreateTextureFromSurface(renderer,ps);
    SDL_Rect pr = {b2.rect.x + b2.rect.w/2 - ps->w/2,
                   b2.rect.y + b2.rect.h/2 - ps->h/2,
                   ps->w, ps->h};
    SDL_FreeSurface(ps);

    SDL_Surface *qs2 = TTF_RenderText_Blended(fontBtn,"Quitter",black);
    SDL_Texture *qt2 = SDL_CreateTextureFromSurface(renderer,qs2);
    SDL_Rect qr2 = {b3.rect.x + b3.rect.w/2 - qs2->w/2,
                    b3.rect.y + b3.rect.h/2 - qs2->h/2,
                    qs2->w, qs2->h};
    SDL_FreeSurface(qs2);

    float time = 0;

    while(running)
    {
        while(SDL_PollEvent(&e))
        {
            if(e.type==SDL_QUIT) running=0;

            int mx,my;
            SDL_GetMouseState(&mx,&my);

            updateButtonHover(&b1,mx,my);
            updateButtonHover(&b2,mx,my);
            updateButtonHover(&b3,mx,my);

            if(e.type==SDL_MOUSEBUTTONDOWN)
            {
                if(b1.isHovered){Mix_PlayChannel(-1,click,0); runQuiz(renderer);}
                if(b3.isHovered){Mix_PlayChannel(-1,click,0); running=0;}
            }
        }

        time += 0.05;

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer,bg,NULL,NULL);

        // 🌑 overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer,0,0,0,120);
        SDL_Rect ov={0,0,W,H};
        SDL_RenderFillRect(renderer,&ov);

        // ✨ TITLE ANIMATION (fade + floating)
        int alpha = (sin(time)+1)*127;
        SDL_SetTextureAlphaMod(tt, alpha);
        tr.y = 50 + sin(time)*10;
        SDL_RenderCopy(renderer,tt,NULL,&tr);

        // 🔘 BUTTONS
        renderButton(&b1,renderer);
        renderButton(&b2,renderer);
        renderButton(&b3,renderer);

        // ✍️ TEXT
        SDL_RenderCopy(renderer,qt,NULL,&qr);
        SDL_RenderCopy(renderer,pt,NULL,&pr);
        SDL_RenderCopy(renderer,qt2,NULL,&qr2);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    Mix_FreeMusic(music);
}

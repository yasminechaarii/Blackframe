#include "quiz.h"
#include "button.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

static int answered = 0;
static int correct = -1;

void runQuiz(SDL_Renderer *renderer)
{
    // Reset
    answered = 0;
    correct = -1;

    SDL_Event e;
    int running = 1;
    int W = 1000, H = 700;

    SDL_Texture *bg = IMG_LoadTexture(renderer, "assets/background.png");

    TTF_Init();
    TTF_Font *font = TTF_OpenFont("assets/font.ttf", 28);

    SDL_Color white = {255,255,255};
    SDL_Color black = {0,0,0};

    // ===== QUESTION (2 LINES) =====
    SDL_Surface *qs1 = TTF_RenderText_Blended(font,
        "Why does Ethan Hunt go on the run",
        white);

    SDL_Surface *qs2 = TTF_RenderText_Blended(font,
        "in Mission: Impossible (1996)?",
        white);

    SDL_Texture *qt1 = SDL_CreateTextureFromSurface(renderer, qs1);
    SDL_Texture *qt2 = SDL_CreateTextureFromSurface(renderer, qs2);

    SDL_Rect qr1 = {W/2 - qs1->w/2, 80, qs1->w, qs1->h};
    SDL_Rect qr2 = {W/2 - qs2->w/2, 130, qs2->w, qs2->h};

    SDL_FreeSurface(qs1);
    SDL_FreeSurface(qs2);

    // ===== ANSWERS =====
    char *answers[3] = {
        "A) Because he steals secret gadgets",
        "B) Because he is framed for a failed mission",
        "C) Because he wants to leave the agency"
    };

    SDL_Texture *txtQ[3];
    SDL_Rect rectQ[3];

    for(int i=0;i<3;i++){
        SDL_Surface *s = TTF_RenderText_Blended(font, answers[i], white);
        txtQ[i] = SDL_CreateTextureFromSurface(renderer, s);
        rectQ[i] = (SDL_Rect){ W/2 - s->w/2, 250 + i*70, s->w, s->h };
        SDL_FreeSurface(s);
    }

    // ===== BUTTONS A B C =====
    SDL_Texture *btn = IMG_LoadTexture(renderer, "assets/retour.png");
    SDL_Texture *btnh = IMG_LoadTexture(renderer, "assets/hover_retour.png");

    Button b[3];
    SDL_Texture *txt[3];
    SDL_Rect rectTxt[3];
    char *letters[3] = {"A","B","C"};

    int spacing = 60;
    int btnW = 180;
    int btnH = 80;
    int totalWidth = 3*btnW + 2*spacing;
    int startX = (W - totalWidth)/2;
    int y = H - 150;

    for(int i=0;i<3;i++){
        initButton(&b[i], startX + i*(btnW + spacing), y, btnW, btnH, btn, btnh);

        SDL_Surface *s = TTF_RenderText_Blended(font, letters[i], black);
        txt[i] = SDL_CreateTextureFromSurface(renderer, s);

        rectTxt[i].w = s->w;
        rectTxt[i].h = s->h;
        rectTxt[i].x = b[i].rect.x + b[i].rect.w/2 - s->w/2;
        rectTxt[i].y = b[i].rect.y + b[i].rect.h/2 - s->h/2;

        SDL_FreeSurface(s);
    }

    // ===== RETOUR BUTTON =====
    SDL_Texture *ret = IMG_LoadTexture(renderer,"assets/retour.png");
    SDL_Texture *reth = IMG_LoadTexture(renderer,"assets/hover_retour.png");

    SDL_Surface *rets = TTF_RenderText_Blended(font, "Retour", black);
    SDL_Texture *retTxt = SDL_CreateTextureFromSurface(renderer, rets);

    SDL_Rect rectRetTxt = {
        20 + 150/2 - rets->w/2,
        20 + 60/2 - rets->h/2,
        rets->w,
        rets->h
    };
    SDL_FreeSurface(rets);

    Button back;
    initButton(&back,20,20,150,60,ret,reth);

    // ===== LOOP =====
    while(running){
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) running=0;

            int mx,my;
            SDL_GetMouseState(&mx,&my);

            for(int i=0;i<3;i++) updateButtonHover(&b[i], mx, my);
            updateButtonHover(&back, mx, my);

            if(e.type==SDL_MOUSEBUTTONDOWN){

                if(back.isHovered)
                    running = 0;

                for(int i=0;i<3;i++){
                    if(b[i].isHovered){

                        if(i == 1){ 
                            // ✅ CORRECT
                            answered = 1;
                            correct = 1;

                        } else {
                            // ❌ WRONG
                            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                            SDL_SetRenderDrawColor(renderer, 255,0,0,180);
                            SDL_Rect flash = {0,0,W,H};
                            SDL_RenderFillRect(renderer,&flash);
                            SDL_RenderPresent(renderer);
                            SDL_Delay(500);

                            running = 0;
                        }
                    }
                }
            }
        }

        // BLACK SCREEN IF CORRECT
        if(answered && correct){
            SDL_SetRenderDrawColor(renderer, 0,0,0,255);
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);
            continue;
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bg, NULL, NULL);

        // QUESTION (2 lines)
        SDL_RenderCopy(renderer, qt1, NULL, &qr1);
        SDL_RenderCopy(renderer, qt2, NULL, &qr2);

        // ANSWERS TEXT
        for(int i=0;i<3;i++)
            SDL_RenderCopy(renderer, txtQ[i], NULL, &rectQ[i]);

        // BUTTONS
        for(int i=0;i<3;i++){
            renderButton(&b[i], renderer);
            SDL_RenderCopy(renderer, txt[i], NULL, &rectTxt[i]);
        }

        // RETOUR
        renderButton(&back, renderer);
        SDL_RenderCopy(renderer, retTxt, NULL, &rectRetTxt);

        SDL_RenderPresent(renderer);
    }

    // ===== CLEANUP =====
    for(int i=0;i<3;i++){
        SDL_DestroyTexture(txt[i]);
        SDL_DestroyTexture(txtQ[i]);
    }

    SDL_DestroyTexture(qt1);
    SDL_DestroyTexture(qt2);

    SDL_DestroyTexture(btn);
    SDL_DestroyTexture(btnh);
    SDL_DestroyTexture(bg);
    SDL_DestroyTexture(ret);
    SDL_DestroyTexture(reth);
    SDL_DestroyTexture(retTxt);

    TTF_CloseFont(font);
}

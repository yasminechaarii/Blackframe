#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

#include "background.h"
#include "player.h"

#define HS_MAX 10
#define HS_FILE "highscores.txt"

typedef struct {
    char name[32];
    int score;
} HighScore;

 HighScore hs_table[HS_MAX];
 int hs_count = 0;
 int hs_state = 0; 
 char hs_input[32] = "";
 int hs_input_len = 0;
 int show_guide = 0;

                     
 int p2_left  = 0;   
 int p2_right = 0;   
 int p2_jump  = 0;   


 void draw_text(SDL_Renderer* renderer, TTF_Font* font,
                      const char* text, int x, int y, SDL_Color color)
{
    SDL_Surface* s = TTF_RenderUTF8_Blended(font, text, color);
    if (!s) return;
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
    if (!t) { SDL_FreeSurface(s); return; }
    SDL_Rect dst = {x, y, s->w, s->h};
    SDL_RenderCopy(renderer, t, NULL, &dst);
    SDL_DestroyTexture(t);
    SDL_FreeSurface(s);
}


 void hs_load(void)
{
    FILE* f = fopen(HS_FILE, "r");
    if (!f) return;
    hs_count = 0;
    while (hs_count < HS_MAX &&
           fscanf(f, "%31s %d",
                  hs_table[hs_count].name,
                  &hs_table[hs_count].score) == 2)
        hs_count++;
    fclose(f);
}

 void hs_save(void)
{
    FILE* f = fopen(HS_FILE, "w");
    if (!f) return;
    for (int i = 0; i < hs_count; i++)
        fprintf(f, "%s %d\n", hs_table[i].name, hs_table[i].score);
    fclose(f);
}

 void hs_insert(const char* name, int score)
{
    if (hs_count < HS_MAX) hs_count++;
    int i = hs_count - 1;
    hs_table[i].score = score;
    strncpy(hs_table[i].name, name, sizeof(hs_table[i].name) - 1);
    hs_table[i].name[sizeof(hs_table[i].name) - 1] = '\0';
    while (i > 0 && hs_table[i].score > hs_table[i-1].score) {
        HighScore tmp = hs_table[i];
        hs_table[i]   = hs_table[i-1];
        hs_table[i-1] = tmp;
        i--;
    }
    if (hs_count > HS_MAX) hs_count = HS_MAX;
    hs_save();
}


 void draw_guide(SDL_Renderer* ren, TTF_Font* font)
{
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 210);
    SDL_Rect box = {80, 50, SCREEN_W - 160, SCREEN_H - 100};
    SDL_RenderFillRect(ren, &box);

    SDL_Color y = {255, 220, 50,  255};
    SDL_Color w = {255, 255, 255, 255};
    int lx = box.x + 30;
    int ly = box.y + 20;
    int ls = 28;

    draw_text(ren, font, "=== CONTROLS ===",                          lx, ly,       y);
    draw_text(ren, font, "--- Single player ---",                    lx, ly+ls*1,  y);
    draw_text(ren, font, "Arrow Left / Right  :  Move",              lx, ly+ls*2,  w);
    draw_text(ren, font, "Up Arrow            :  Jump",              lx, ly+ls*3,  w);
    draw_text(ren, font, "--- Split screen (V) ---",                 lx, ly+ls*4,  y);
    draw_text(ren, font, "P1 (right side): Arrow Left/Right + Up",  lx, ly+ls*5,  w);
    draw_text(ren, font, "P2 (left  side): Q = Left  D = Right  Z = Jump", lx, ly+ls*6, w);
    draw_text(ren, font, "--- General ---",                          lx, ly+ls*7,  y);
    draw_text(ren, font, "V   :  Toggle split screen",               lx, ly+ls*8,  w);
    draw_text(ren, font, "R   :  Restart level",                     lx, ly+ls*9,  w);
    draw_text(ren, font, "S   :  Share screen (screenshot)",         lx, ly+ls*10, w);
    draw_text(ren, font, "H   :  High scores",                       lx, ly+ls*11, w);
    draw_text(ren, font, "1 / 2  :  Switch level",                   lx, ly+ls*12, w);
    draw_text(ren, font, "ESC :  Quit",                              lx, ly+ls*13, w);
    draw_text(ren, font, "=== HAZARDS ===",                          lx, ly+ls*14, y);
    draw_text(ren, font, "Yellow electricity  ->  instant death",    lx, ly+ls*15, w);
    draw_text(ren, font, "Maroon triangle     ->  instant death",    lx, ly+ls*16, w);
    draw_text(ren, font, "Collect the KEY  ->  door opens!",         lx, ly+ls*17, y);
    draw_text(ren, font, "Press F1 to close",                        lx, ly+ls*18, y);
}


 void draw_hs_menu(SDL_Renderer* ren, TTF_Font* font, int player_score)
{
    (void)player_score;
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, 0, 0, 50, 220);
    SDL_Rect box = {150, 80, 700, 340};
    SDL_RenderFillRect(ren, &box);

    SDL_Color w = {255, 255, 255, 255};
    SDL_Color y = {255, 220,  50, 255};

    if (hs_state == 1) {
        char buf[64];
        draw_text(ren, font, "Enter your name:", 200, 120, y);
        snprintf(buf, sizeof(buf), "> %s_", hs_input);
        draw_text(ren, font, buf, 200, 160, w);
        draw_text(ren, font, "Press ENTER to confirm", 200, 200, w);
    } else if (hs_state == 2) {
        draw_text(ren, font, "=== HIGH SCORES ===", 320, 100, y);
        for (int i = 0; i < hs_count; i++) {
            char line[96];
            snprintf(line, sizeof(line), "%d. %-20s %d",
                     i+1, hs_table[i].name, hs_table[i].score);
            draw_text(ren, font, line, 200, 150 + i*26, w);
        }
        draw_text(ren, font, "F3 / ESC to close", 200, 390, w);
    }
}


 void update_camera_for_view(Camera* cam, const Player* player,
                                   int view_w, int view_h)
{
    cam->rect.w = (float)view_w;
    cam->rect.h = (float)view_h;
    cam->rect.x = player->rect.x + player->rect.w * 0.5f - cam->rect.w * 0.5f;
    cam->rect.y = player->rect.y + player->rect.h * 0.5f - cam->rect.h * 0.5f;

    if (WORLD_W <= view_w) cam->rect.x = 0;
    else {
        if (cam->rect.x < 0) cam->rect.x = 0;
        if (cam->rect.x + cam->rect.w > WORLD_W) cam->rect.x = WORLD_W - cam->rect.w;
    }
    if (WORLD_H <= view_h) cam->rect.y = 0;
    else {
        if (cam->rect.y < 0) cam->rect.y = 0;
        if (cam->rect.y + cam->rect.h > WORLD_H) cam->rect.y = WORLD_H - cam->rect.h;
    }
}


 void handle_input_p1(Player* p, const Uint8* keys)
{
    const float speed = 320.0f;
    p->vx = 0.0f;
    if (keys[SDL_SCANCODE_LEFT])  p->vx = -speed;
    if (keys[SDL_SCANCODE_RIGHT]) p->vx =  speed;
    if (keys[SDL_SCANCODE_UP] && p->on_ground) {
        p->vy = -620.0f;
        p->on_ground = 0;
    }
}


 void handle_input_p2(Player* p, const Uint8* keys)
{
    (void)keys;  
    const float speed = 320.0f;
    p->vx = 0.0f;
    if (p2_left)  p->vx = -speed;
    if (p2_right) p->vx =  speed;
    if (p2_jump && p->on_ground) {
        p->vy    = -620.0f;
        p->on_ground = 0;
        p2_jump  = 0;  
    }
}


 int calc_score(const Player* p, const Level* level,
                      int elapsed_s, int level_id)
{
    int progress    = (int)(p->rect.x * 0.1f);
    int key_bonus   = level->has_key ? 500 : 0;
    int level_bonus = (level_id - 1) * 1200;
    int time_bonus  = elapsed_s * 5;
    return progress + key_bonus + level_bonus + time_bonus;
}


int main(int argc, char** argv)
{
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError()); return 1;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        printf("IMG_Init failed: %s\n", IMG_GetError()); SDL_Quit(); return 1;
    }
    if (TTF_Init() != 0) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        IMG_Quit(); SDL_Quit(); return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "SDL2 Platformer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!window || !renderer) {
        printf("Window/Renderer failed: %s\n", SDL_GetError());
        TTF_Quit(); IMG_Quit(); SDL_Quit(); return 1;
    }

    
    TTF_Font* font = TTF_OpenFont("assets/font.ttf", 20);
    if (!font) font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 20);
    if (!font) font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20);
    if (!font) font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSansBold.ttf", 20);
    if (!font) font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 20);
    if (!font) font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 20);
    if (!font) font = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf", 20);
    if (!font) printf("WARNING: no font found – HUD text will be invisible.\n");
    else       printf("Font loaded OK.\n");

    Background bg = {0};
    if (!background_init(renderer, &bg)) {
        printf("Failed to load textures from assets/.\n");
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
        TTF_Quit(); IMG_Quit(); SDL_Quit(); return 1;
    }

    Level level  = {0};
    int level_id = 1;
    level_load(&level, level_id);

    Player player1, player2;
    player_init(&player1, (float)level.spawn_x, (float)level.spawn_y);
    player_init(&player2, (float)level.spawn_x + 56.0f, (float)level.spawn_y);

    Camera cam1 = {0}, cam2 = {0};
    update_camera_for_view(&cam1, &player1, SCREEN_W, SCREEN_H);
    update_camera_for_view(&cam2, &player2, SCREEN_W, SCREEN_H);

   
    Uint32 start_ticks = SDL_GetTicks();  
    Uint32 last_ticks  = start_ticks;

    hs_load();

    int split_screen = 0;
    int running      = 1;

    while (running)
    {
        
        Uint32 now = SDL_GetTicks();
        float dt = (float)(now - last_ticks) / 1000.0f;
        if (dt > 0.05f) dt = 0.05f;
        last_ticks = now;

        
        int elapsed_s    = (int)((SDL_GetTicks() - start_ticks) / 1000);
        int current_score = calc_score(&player1, &level, elapsed_s, level_id);

       
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT) { running = 0; break; }

            
            if (hs_state == 1 && ev.type == SDL_TEXTINPUT) {
                size_t cur = strlen(hs_input);
                if (cur < 31) {
                    size_t room = 31 - cur;
                    strncat(hs_input, ev.text.text, room);
                    hs_input_len = (int)strlen(hs_input);
                }
            }

            
            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_q) p2_left  = 1;
                if (ev.key.keysym.sym == SDLK_d) p2_right = 1;
                if (ev.key.keysym.sym == SDLK_z) p2_jump  = 1;
            }
            if (ev.type == SDL_KEYUP) {
                if (ev.key.keysym.sym == SDLK_q) p2_left  = 0;
                if (ev.key.keysym.sym == SDLK_d) p2_right = 0;
                if (ev.key.keysym.sym == SDLK_z) p2_jump  = 0;
            }

            if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0) {
                SDL_Keycode k = ev.key.keysym.sym;

                
                if (hs_state == 1) {
                    if (k == SDLK_BACKSPACE && hs_input_len > 0)
                        hs_input[--hs_input_len] = '\0';
                    if (k == SDLK_RETURN && hs_input_len > 0) {
                        hs_insert(hs_input, current_score);
                        hs_state = 2;
                        SDL_StopTextInput();
                    }
                    if (k == SDLK_ESCAPE) {
                        hs_state = 0;
                        SDL_StopTextInput();
                    }
                    continue;
                }

               
                if (hs_state == 2) {
                    if (k == SDLK_ESCAPE || k == SDLK_F3 ||
                        k == SDLK_h      || k == SDLK_c)
                        hs_state = 0;
                    continue;
                }

               
                switch (k)
                {
                case SDLK_ESCAPE: running = 0; break;

              
                case SDLK_F1:
                    show_guide = !show_guide;
                    break;

               
                case SDLK_F2:
                case SDLK_s:
                    screen_share(renderer);
                    break;

               
                case SDLK_F3:
                case SDLK_h:
                    hs_load();
                    hs_state = 1;
                    hs_input[0] = '\0';
                    hs_input_len = 0;
                    SDL_StartTextInput();
                    break;

                
                case SDLK_v:
                    split_screen = !split_screen;
                    break;

                
                case SDLK_r:
                    level_load(&level, level_id);
                    player_init(&player1, (float)level.spawn_x, (float)level.spawn_y);
                    player_init(&player2, (float)level.spawn_x + 56.0f, (float)level.spawn_y);
                    start_ticks = SDL_GetTicks();
                    break;

                
                case SDLK_1:
                    level_id = 1;
                    level_load(&level, level_id);
                    player_init(&player1, (float)level.spawn_x, (float)level.spawn_y);
                    player_init(&player2, (float)level.spawn_x + 56.0f, (float)level.spawn_y);
                    start_ticks = SDL_GetTicks();
                    break;
                case SDLK_2:
                    level_id = 2;
                    level_load(&level, level_id);
                    player_init(&player1, (float)level.spawn_x, (float)level.spawn_y);
                    player_init(&player2, (float)level.spawn_x + 56.0f, (float)level.spawn_y);
                    start_ticks = SDL_GetTicks();
                    break;

                default: break;
                }
            }
        }

        
        if (hs_state == 0 && !show_guide)
        {
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            level_update(&level, dt);

            if (split_screen) {
                handle_input_p1(&player1, keys);
                handle_input_p2(&player2, keys);
                player_update(&player1, &level, dt);
                player_update(&player2, &level, dt);
            } else {
                player_handle_input(&player1, keys);
                player_update(&player1, &level, dt);
            }

            
            if (level_try_take_key(&level, player1.rect) ||
                (split_screen && level_try_take_key(&level, player2.rect)))
                level.has_key = 1;

           
            if (level_player_hits_hazard(&level, player1.rect)) {
                player_init(&player1, (float)level.spawn_x, (float)level.spawn_y);
                start_ticks = SDL_GetTicks();
            }
            if (split_screen && level_player_hits_hazard(&level, player2.rect))
                player_init(&player2, (float)level.spawn_x + 56.0f, (float)level.spawn_y);

            
            if (level_try_enter_door(&level, player1.rect) ||
                (split_screen && level_try_enter_door(&level, player2.rect)))
            {
                if (level_id == 1) {
                    level_id = 2;
                    level_load(&level, level_id);
                    player_init(&player1, (float)level.spawn_x, (float)level.spawn_y);
                    player_init(&player2, (float)level.spawn_x + 56.0f, (float)level.spawn_y);
                    start_ticks = SDL_GetTicks();
                }
            }
        }

       
        SDL_SetRenderDrawColor(renderer, 223, 180, 134, 255);
        SDL_RenderClear(renderer);

        if (split_screen) {
            SDL_Rect lv = {0,           0, SCREEN_W/2,          SCREEN_H};
            SDL_Rect rv = {SCREEN_W/2,  0, SCREEN_W - SCREEN_W/2, SCREEN_H};

            update_camera_for_view(&cam1, &player2, lv.w, lv.h);  
            update_camera_for_view(&cam2, &player1, rv.w, rv.h);  

            
            SDL_RenderSetViewport(renderer, &lv);
            background_draw(renderer, &bg, &level, &cam1);
            player_draw(renderer, &player2, &cam1);

           
            SDL_RenderSetViewport(renderer, &rv);
            background_draw(renderer, &bg, &level, &cam2);
            player_draw(renderer, &player1, &cam2);

            SDL_RenderSetViewport(renderer, NULL);
           
            SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
            SDL_RenderDrawLine(renderer, SCREEN_W/2, 0, SCREEN_W/2, SCREEN_H);
            
            if (font) {
                SDL_Color p2col = {100, 200, 255, 255};
                SDL_Color p1col = {255, 160,  60, 255};
                draw_text(renderer, font, "P2  Q<  D>  Z jump", 8,            8, p2col);
                draw_text(renderer, font, "P1  <  >  Up jump",  SCREEN_W/2+8, 8, p1col);
            }
        } else {
            update_camera_for_view(&cam1, &player1, SCREEN_W, SCREEN_H);
            SDL_RenderSetViewport(renderer, NULL);
            background_draw(renderer, &bg, &level, &cam1);
            player_draw(renderer, &player1, &cam1);
        }

      
        if (font) {
            int mm = elapsed_s / 60;
            int ss = elapsed_s % 60;
            char hud_time[32], hud_score[32];
            snprintf(hud_time,  sizeof(hud_time),  "Time  %02d:%02d", mm, ss);
            snprintf(hud_score, sizeof(hud_score), "Score %d", current_score);

            SDL_Color white  = {255, 255, 255, 255};
            SDL_Color yellow = {255, 220,  50, 255};

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 140);
            SDL_Rect hud_box = {0, 0, 200, 58};
            SDL_RenderFillRect(renderer, &hud_box);

            draw_text(renderer, font, hud_time,  10, 8,  white);
            draw_text(renderer, font, hud_score, 10, 32, white);

           
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
            SDL_Rect bot_box = {0, SCREEN_H - 34, SCREEN_W, 34};
            SDL_RenderFillRect(renderer, &bot_box);

           
            int bw = SCREEN_W / 4;
            draw_text(renderer, font, "R",            bw*0 + 14,  SCREEN_H - 26, yellow);
            draw_text(renderer, font, "Restart",      bw*0 + 28,  SCREEN_H - 26, white);

            draw_text(renderer, font, "H",            bw*1 + 14,  SCREEN_H - 26, yellow);
            draw_text(renderer, font, "High Scores",  bw*1 + 28,  SCREEN_H - 26, white);

            draw_text(renderer, font, "S",            bw*2 + 14,  SCREEN_H - 26, yellow);
            draw_text(renderer, font, "Share Screen", bw*2 + 28,  SCREEN_H - 26, white);

            draw_text(renderer, font, "V",            bw*3 + 14,  SCREEN_H - 26, yellow);
            draw_text(renderer, font, "Split Screen", bw*3 + 28,  SCREEN_H - 26, white);
        }

        
        if (show_guide && font)
            draw_guide(renderer, font);

       
        if (hs_state && font)
            draw_hs_menu(renderer, font, current_score);

        SDL_RenderPresent(renderer);
    }

    background_destroy(&bg);
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}

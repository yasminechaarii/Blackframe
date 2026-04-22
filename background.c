#include "background.h"

#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "player.h"

 SDL_Surface* g_road_mask  = NULL;
 SDL_Surface* g_road2_mask = NULL;

 SDL_Texture* load_tex(SDL_Renderer* r, const char* path) {
    SDL_Surface* s = IMG_Load(path);
    if (!s) {
        printf("IMG_Load failed (%s): %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
    SDL_FreeSurface(s);
    return t;
}

 int overlap(SDL_FRect a, SDL_FRect b) {
    return !(a.x + a.w <= b.x || b.x + b.w <= a.x || a.y + a.h <= b.y || b.y + b.h <= a.y);
}


 int road_point_solid_mask(SDL_Surface* mask, float wx, float wy) {
    if (!mask) return 0;
    if (wx < ROAD_DRAW_X || wy < ROAD_DRAW_Y) return 0;
    if (wx >= ROAD_DRAW_X + ROAD_DRAW_W || wy >= ROAD_DRAW_Y + ROAD_DRAW_H) return 0;

    float u = (wx - ROAD_DRAW_X) / (float)ROAD_DRAW_W;
    float v = (wy - ROAD_DRAW_Y) / (float)ROAD_DRAW_H;
    int px = (int)(u * (float)mask->w);
    int py = (int)(v * (float)mask->h);

    if (px < 0) px = 0;
    if (py < 0) py = 0;
    if (px >= mask->w) px = mask->w - 1;
    if (py >= mask->h) py = mask->h - 1;

    uint32_t* pixels = (uint32_t*)mask->pixels;
    uint32_t p = pixels[py * (mask->pitch / 4) + px];
    Uint8 r, g, b, a;
    SDL_GetRGBA(p, mask->format, &r, &g, &b, &a);
    int luminance = (int)(0.299f * (float)r + 0.587f * (float)g + 0.114f * (float)b);
    return a > 20 && luminance < 70;
}


 SDL_Surface* g_active_mask = NULL;

 int road_point_solid(float wx, float wy) {
    return road_point_solid_mask(g_active_mask, wx, wy);
}

 float road_top_y(float world_x) {
    for (int y = 0; y < WORLD_H; y++) {
        if (road_point_solid(world_x, (float)y)) return (float)y;
    }
    return -1.0f;
}

 SDL_FRect place_on_road(float x, float w, float h, float raise) {
    float top = road_top_y(x + w * 0.5f);
    if (top < 0.0f) top = (float)(ROAD_DRAW_Y + ROAD_DRAW_H);
    return (SDL_FRect){x, top - h - raise, w, h};
}

int road_rect_collides(SDL_FRect rect) {
    int min_x = (int)floorf(rect.x);
    int max_x = (int)ceilf(rect.x + rect.w);
    int min_y = (int)floorf(rect.y);
    int max_y = (int)ceilf(rect.y + rect.h);

    for (int y = min_y; y <= max_y; y += 3)
        for (int x = min_x; x <= max_x; x += 3)
            if (road_point_solid((float)x, (float)y)) return 1;

    for (int x = min_x; x <= max_x; x++)
        if (road_point_solid((float)x, rect.y) ||
            road_point_solid((float)x, rect.y + rect.h - 1.0f)) return 1;
    for (int y = min_y; y <= max_y; y++)
        if (road_point_solid(rect.x, (float)y) ||
            road_point_solid(rect.x + rect.w - 1.0f, (float)y)) return 1;
    return 0;
}

int level_solid_rect_collides(const Level* level, SDL_FRect rect) {
    if (road_rect_collides(rect)) return 1;
    for (int i = 0; i < level->hazard_count; i++) {
        const Hazard* hz = &level->hazards[i];
        if (hz->type == HAZARD_DASH && overlap(rect, hz->rect)) return 1;
    }
    return 0;
}

 void add_hazard(Level* l, HazardType t, SDL_FRect r,
                       int axis, float speed, float range, float phase) {
    if (l->hazard_count >= MAX_HAZARDS) return;
    Hazard* hz = &l->hazards[l->hazard_count++];
    hz->rect      = r;
    hz->base_rect = r;
    hz->type  = t;
    hz->axis  = axis;
    hz->speed = speed;
    hz->range = range;
    hz->phase = phase;
}


int background_init(SDL_Renderer* renderer, Background* bg) {
    bg->bg_level1 = load_tex(renderer, "assets/bg_level1.png");
    bg->bg_level2 = load_tex(renderer, "assets/bg_level2.png");

    
    SDL_Surface* road_src = IMG_Load("assets/road.png");
    if (road_src) {
        bg->road  = SDL_CreateTextureFromSurface(renderer, road_src);
        g_road_mask = SDL_ConvertSurfaceFormat(road_src, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(road_src);
    }

    SDL_Surface* road2_src = IMG_Load("assets/road2.png");
    if (road2_src) {
        bg->road2  = SDL_CreateTextureFromSurface(renderer, road2_src);
        g_road2_mask = SDL_ConvertSurfaceFormat(road2_src, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(road2_src);
    } else {
      
        bg->road2    = bg->road;
        g_road2_mask = g_road_mask;
        printf("Warning: assets/road2.png not found, using road.png for level 2.\n");
    }

    bg->door_closed = load_tex(renderer, "assets/door_closed.png");
    bg->door_open   = load_tex(renderer, "assets/door_open.png");
    bg->chest       = load_tex(renderer, "assets/chest.png");
    bg->key         = load_tex(renderer, "assets/key.png");
    bg->dash        = load_tex(renderer, "assets/dash.png");
    bg->electric    = load_tex(renderer, "assets/electric.png");
    bg->triangle    = load_tex(renderer, "assets/triangle.png");

    
    g_active_mask = g_road_mask;

    return bg->bg_level1 && bg->road;
}


void background_destroy(Background* bg) {
    SDL_DestroyTexture(bg->bg_level1);
    SDL_DestroyTexture(bg->bg_level2);
    SDL_DestroyTexture(bg->road);
  
    if (bg->road2 && bg->road2 != bg->road)
        SDL_DestroyTexture(bg->road2);
    SDL_DestroyTexture(bg->door_closed);
    SDL_DestroyTexture(bg->door_open);
    SDL_DestroyTexture(bg->chest);
    SDL_DestroyTexture(bg->key);
    SDL_DestroyTexture(bg->dash);
    SDL_DestroyTexture(bg->electric);
    SDL_DestroyTexture(bg->triangle);

    if (g_road_mask) { SDL_FreeSurface(g_road_mask);  g_road_mask  = NULL; }
    if (g_road2_mask && g_road2_mask != g_road_mask)
        { SDL_FreeSurface(g_road2_mask); g_road2_mask = NULL; }
    g_active_mask = NULL;
}


void level_load(Level* l, int id) {
    *l = (Level){0};
    l->id       = id;
    l->world    = (SDL_FRect){0, 0, WORLD_W, WORLD_H};
    l->ground_y = ROAD_DRAW_Y;
    l->ground_h = ROAD_DRAW_H;
    l->has_key  = 0;

    
    g_active_mask = (id == 2) ? g_road2_mask : g_road_mask;

    l->spawn_x = 8;
    l->spawn_y = (int)place_on_road((float)l->spawn_x, 40.0f, 40.0f, 0.0f).y;

    if (id == 1) {
        l->door  = (SDL_FRect){9860.0f, 246.0f, 64.0f, 92.0f};
        l->chest = (SDL_FRect){1932.0f, 286.0f, 48.0f, 40.0f};
        l->key   = (SDL_FRect){1940.0f, 236.0f, 34.0f, 34.0f};

        add_hazard(l, HAZARD_ELECTRIC, (SDL_FRect){1030.0f, 433.0f, 250.0f, 26.0f}, 0, 0.0f,  0.0f, 0.0f);
        add_hazard(l, HAZARD_DASH,     (SDL_FRect){1050.0f, 320.0f, 168.0f, 22.0f}, 1, 1.35f, 14.0f, 0.0f);
        add_hazard(l, HAZARD_DASH,     (SDL_FRect){2300.0f, 320.0f, 164.0f, 22.0f}, 0, 1.1f,  38.0f, 0.5f);
        add_hazard(l, HAZARD_ELECTRIC, (SDL_FRect){3000.0f, 433.0f, 250.0f, 26.0f}, 0, 0.0f,  0.0f, 0.0f);
        add_hazard(l, HAZARD_TRIANGLE, (SDL_FRect){5500.0f, 420.0f, 100.0f, 30.0f}, 0, 0.0f,  0.0f, 0.0f);
        add_hazard(l, HAZARD_TRIANGLE, (SDL_FRect){5800.0f, 420.0f, 100.0f, 30.0f}, 0, 0.0f,  0.0f, 0.0f);
        add_hazard(l, HAZARD_TRIANGLE, (SDL_FRect){6100.0f, 420.0f, 100.0f, 30.0f}, 0, 0.0f,  0.0f, 0.0f);
        add_hazard(l, HAZARD_TRIANGLE, (SDL_FRect){6400.0f, 420.0f, 100.0f, 30.0f}, 0, 0.0f,  0.0f, 0.0f);
        add_hazard(l, HAZARD_DASH,     (SDL_FRect){1900.0f, 330.0f, 140.0f, 22.0f}, 1, 1.25f, 18.0f, 0.2f);
        add_hazard(l, HAZARD_DASH,     (SDL_FRect){3100.0f, 320.0f, 140.0f, 22.0f}, 1, 1.25f, 18.0f, 0.2f);
        add_hazard(l, HAZARD_DASH,     (SDL_FRect){6800.0f, 320.0f, 140.0f, 22.0f}, 1, 1.3f,  24.0f, 0.7f);
        add_hazard(l, HAZARD_DASH,     (SDL_FRect){7650.0f, 320.0f, 140.0f, 22.0f}, 1, 1.2f,  20.0f, 0.4f);
        add_hazard(l, HAZARD_DASH,     (SDL_FRect){8400.0f, 320.0f, 140.0f, 22.0f}, 1, 1.2f,  20.0f, 0.4f);
        add_hazard(l, HAZARD_DASH,     (SDL_FRect){9250.0f, 320.0f, 140.0f, 22.0f}, 1, 1.2f,  20.0f, 0.4f);

    } else {
       
        l->chest = (SDL_FRect){5000.0f, 100.0f, 34.0f, 36.0f};
        l->key   = place_on_road(4000.0f, 34.0f, 34.0f, 36.0f);
        l->door  = place_on_road(9700.0f, 64.0f, 110.0f, 180.0f);

        
        add_hazard(l, HAZARD_DASH, place_on_road(6500.0f, 200.0f, 22.0f, 0.0f), 0, 1.2f,  90.0f, 0.0f);
        add_hazard(l, HAZARD_DASH, place_on_road(6700.0f, 150.0f, 22.0f, 0.0f), 1, 1.4f,  60.0f, 0.3f);
        add_hazard(l, HAZARD_DASH, place_on_road(6900.0f, 150.0f, 22.0f, 0.0f), 0, 1.0f,  80.0f, 0.6f);
        add_hazard(l, HAZARD_DASH, place_on_road(7100.0f, 150.0f, 22.0f, 0.0f), 1, 1.3f,  70.0f, 0.1f);
        add_hazard(l, HAZARD_DASH, place_on_road(7300.0f, 150.0f, 22.0f, 0.0f), 0, 1.5f, 100.0f, 0.8f);
        add_hazard(l, HAZARD_DASH, place_on_road(7400.0f, 150.0f, 22.0f, 0.0f), 1, 1.1f,  55.0f, 0.4f);

        
        add_hazard(l, HAZARD_TRIANGLE, place_on_road(800.0f, 60.0f, 30.0f, 0.0f), 0, 0.0f, 0.0f, 0.0f);
        add_hazard(l, HAZARD_TRIANGLE, place_on_road(1000.0f, 60.0f, 30.0f, 0.0f), 0, 0.0f, 0.0f, 0.0f);
        add_hazard(l, HAZARD_TRIANGLE, place_on_road(1200.0f, 60.0f, 30.0f, 0.0f), 0, 0.0f, 0.0f, 0.0f);
        add_hazard(l, HAZARD_TRIANGLE, place_on_road(1400.0f, 60.0f, 30.0f, 0.0f), 0, 0.0f, 0.0f, 0.0f);
        add_hazard(l, HAZARD_TRIANGLE, place_on_road(1600.0f, 60.0f, 30.0f, 0.0f), 0, 0.0f, 0.0f, 0.0f);
        add_hazard(l, HAZARD_TRIANGLE, place_on_road(1800.0f, 60.0f, 30.0f, 0.0f), 0, 0.0f, 0.0f, 0.0f);
        add_hazard(l, HAZARD_TRIANGLE, place_on_road(2000.0f, 60.0f, 30.0f, 0.0f), 0, 0.0f, 0.0f, 0.0f);
        add_hazard(l, HAZARD_TRIANGLE, place_on_road(2200.0f, 60.0f, 30.0f, 0.0f), 0, 0.0f, 0.0f, 0.0f);
    }
}

void level_update(Level* l, float dt) {
    l->mover_time += dt;
    for (int i = 0; i < l->hazard_count; i++) {
        Hazard* hz = &l->hazards[i];
        if (hz->type != HAZARD_DASH) continue;
        float offset = sinf(l->mover_time * hz->speed + hz->phase) * hz->range;
        hz->rect = hz->base_rect;
        if (hz->axis == 0) hz->rect.x += offset;
        else               hz->rect.y += offset;
    }
}


void camera_update(Camera* cam, const struct Player* player) {
    cam->rect.w = SCREEN_W;
    cam->rect.h = SCREEN_H;
    cam->rect.x = player->rect.x + player->rect.w * 0.5f - cam->rect.w * 0.5f;
    cam->rect.y = player->rect.y + player->rect.h * 0.5f - cam->rect.h * 0.5f;

    if (WORLD_W <= SCREEN_W) cam->rect.x = 0;
    else {
        if (cam->rect.x < 0) cam->rect.x = 0;
        if (cam->rect.x + cam->rect.w > WORLD_W) cam->rect.x = WORLD_W - cam->rect.w;
    }
    if (WORLD_H <= SCREEN_H) cam->rect.y = 0;
    else {
        if (cam->rect.y < 0) cam->rect.y = 0;
        if (cam->rect.y + cam->rect.h > WORLD_H) cam->rect.y = WORLD_H - cam->rect.h;
    }
}

SDL_FRect cam_rect(const Camera* cam, SDL_FRect world_rect) {
    SDL_FRect r = world_rect;
    r.x -= cam->rect.x;
    r.y -= cam->rect.y;
    return r;
}


void background_draw(SDL_Renderer* renderer, const Background* bg,
                     const Level* level, const Camera* cam) {
    
    SDL_Texture* active_bg = (level->id == 2 && bg->bg_level2) ? bg->bg_level2 : bg->bg_level1;
    SDL_FRect bg_world  = {0, 0, BG_DRAW_W, BG_DRAW_H};
    SDL_FRect bg_screen = cam_rect(cam, bg_world);
    SDL_RenderCopyF(renderer, active_bg, NULL, &bg_screen);

    
    SDL_Texture* active_road = (level->id == 2 && bg->road2) ? bg->road2 : bg->road;
    SDL_FRect road_world  = {ROAD_DRAW_X, ROAD_DRAW_Y, ROAD_DRAW_W, ROAD_DRAW_H};
    SDL_FRect road_screen = cam_rect(cam, road_world);
    SDL_RenderCopyF(renderer, active_road, NULL, &road_screen);

    
    for (int i = 0; i < level->hazard_count; i++) {
        const Hazard* hz = &level->hazards[i];
        SDL_FRect dst = cam_rect(cam, hz->rect);
        SDL_Texture* tex = NULL;
        if (hz->type == HAZARD_DASH)     tex = bg->dash;
        if (hz->type == HAZARD_ELECTRIC) tex = bg->electric;
        if (hz->type == HAZARD_TRIANGLE) tex = bg->triangle;

        if (tex) SDL_RenderCopyF(renderer, tex, NULL, &dst);
        else {
            SDL_SetRenderDrawColor(renderer, 150, 40, 20, 255);
            SDL_RenderFillRectF(renderer, &dst);
        }
    }

  
    if (level->chest.w > 0.0f && bg->chest) {
        SDL_FRect chest_dst = cam_rect(cam, level->chest);
        SDL_RenderCopyF(renderer, bg->chest, NULL, &chest_dst);
    }

   
    if (!level->has_key) {
        SDL_FRect key_dst = cam_rect(cam, level->key);
        if (bg->key) SDL_RenderCopyF(renderer, bg->key, NULL, &key_dst);
    }

  
    SDL_FRect door_dst = cam_rect(cam, level->door);
    SDL_Texture* door_tex = level->has_key ? bg->door_open : bg->door_closed;
    if (door_tex) SDL_RenderCopyF(renderer, door_tex, NULL, &door_dst);
}


int level_player_hits_hazard(const Level* level, SDL_FRect player_rect) {
    for (int i = 0; i < level->hazard_count; i++) {
        const Hazard* hz = &level->hazards[i];
        if (hz->type == HAZARD_DASH) continue;
        if (overlap(player_rect, hz->rect)) return 1;
    }
    return 0;
}

int level_try_take_key(Level* level, SDL_FRect player_rect) {
    if (level->has_key) return 0;
    if (overlap(player_rect, level->key)) {
        level->has_key = 1;
        return 1;
    }
    return 0;
}

int level_try_enter_door(const Level* level, SDL_FRect player_rect) {
    if (!level->has_key) return 0;
    return overlap(player_rect, level->door);
}


void screen_share(SDL_Renderer* renderer) {
    int w = 0, h = 0;
    SDL_GetRendererOutputSize(renderer, &w, &h);
    SDL_Surface* frame = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
    if (!frame) return;
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32,
                             frame->pixels, frame->pitch) == 0) {
        char name[64];
        snprintf(name, sizeof(name), "screenshot_%u.bmp", (unsigned)time(NULL));
        SDL_SaveBMP(frame, name);
        printf("Screenshot saved: %s\n", name);
    }
    SDL_FreeSurface(frame);
}

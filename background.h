#ifndef BACKGROUND_H
#define BACKGROUND_H
#include <SDL2/SDL.h>
#define SCREEN_W 1000
#define SCREEN_H 500
#define WORLD_W 10000
#define WORLD_H 500
#define BG_DRAW_W 10000
#define BG_DRAW_H 500
#define ROAD_DRAW_X 0
#define ROAD_DRAW_W 10000
#define ROAD_DRAW_H 200
#define ROAD_DRAW_Y 300
#define MAX_PLATFORMS 96
#define MAX_HAZARDS 64
typedef enum {
    PLATFORM_SOLID = 0,
    PLATFORM_ONE_WAY,
    PLATFORM_BOOST
} PlatformType;
typedef enum {
    HAZARD_DASH = 0,
    HAZARD_ELECTRIC,
    HAZARD_TRIANGLE
} HazardType;
typedef struct {
    SDL_FRect rect;
    PlatformType type;
} Platform;
typedef struct {
    SDL_FRect rect;
    SDL_FRect base_rect;
    HazardType type;
    int axis;
    float speed;
    float range;
    float phase;
} Hazard;
typedef struct {
    int id;
    SDL_FRect world;
    float ground_y;
    float ground_h;
    SDL_FRect door;
    SDL_FRect chest;
    SDL_FRect key;
    int has_key;
    int spawn_x;
    int spawn_y;
    Platform platforms[MAX_PLATFORMS];
    int platform_count;
    Hazard hazards[MAX_HAZARDS];
    int hazard_count;
    float mover_time;
} Level;
typedef struct {
    SDL_FRect rect;
} Camera;
typedef struct {
    SDL_Texture* bg_level1;
    SDL_Texture* bg_level2;
    SDL_Texture* road;
    SDL_Texture* road2;       
    SDL_Texture* door_closed;
    SDL_Texture* door_open;
    SDL_Texture* chest;
    SDL_Texture* key;
    SDL_Texture* dash;
    SDL_Texture* electric;
    SDL_Texture* triangle;
} Background;
struct Player;
int background_init(SDL_Renderer* renderer, Background* bg);
void background_destroy(Background* bg);
void level_load(Level* level, int id);
void level_update(Level* level, float dt);
void camera_update(Camera* cam, const struct Player* player);
SDL_FRect cam_rect(const Camera* cam, SDL_FRect world_rect);
void background_draw(SDL_Renderer* renderer, const Background* bg, const Level* level, const Camera* cam);
int level_player_hits_hazard(const Level* level, SDL_FRect player_rect);
int level_try_take_key(Level* level, SDL_FRect player_rect);
int level_try_enter_door(const Level* level, SDL_FRect player_rect);
void screen_share(SDL_Renderer* renderer);
int road_rect_collides(SDL_FRect rect);
int level_solid_rect_collides(const Level* level, SDL_FRect rect);
#endif

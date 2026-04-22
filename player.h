#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define PLAYER_MAX_FRAMES 12

typedef enum PlayerAction {
    PLAYER_ACTION_WALK = 0,
    PLAYER_ACTION_RUN,
    PLAYER_ACTION_JUMP,
    PLAYER_ACTION_DODGE,
    PLAYER_ACTION_KICK,
    PLAYER_ACTION_PUNCH,
    PLAYER_ACTION_FLYING_KICK,
    PLAYER_ACTION_SHOOT,
    PLAYER_ACTION_CROUCH_SHOT,
    PLAYER_ACTION_POWER_SHOT,
    PLAYER_ACTION_COUNT
} PlayerAction;

typedef struct PlayerInput {
    bool move_left;
    bool move_right;
    bool run;
    bool jump_pressed;
    bool dodge_pressed;
    bool kick_pressed;
    bool punch_pressed;
    bool flying_kick_pressed;
    bool shoot_pressed;
    bool crouch_shot_pressed;
    bool power_shot_pressed;
} PlayerInput;

typedef struct PlayerAnimationClip {
    SDL_Rect frames[PLAYER_MAX_FRAMES];
    int frame_count;
    float fps;
    bool looping;
} PlayerAnimationClip;

typedef struct Player {
    SDL_Texture *texture;
    int texture_width;
    int texture_height;
    float x;
    float y;
    float vx;
    float vy;
    float ground_y;
    bool on_ground;
    bool facing_right;
    PlayerAction action;
    float action_time;
    int frame_index;
    bool shot_emitted;
    uint32_t used_actions_mask;
    PlayerAnimationClip clips[PLAYER_ACTION_COUNT];
} Player;

bool Player_Init(Player *player, SDL_Renderer *renderer, const char *sprite_sheet_path, float ground_y);
void Player_Destroy(Player *player);

void Player_Update(Player *player, const PlayerInput *input, float dt);
void Player_Render(const Player *player, SDL_Renderer *renderer, float camera_x);

PlayerAction Player_GetAction(const Player *player);
const char *Player_GetActionName(PlayerAction action);
uint32_t Player_GetUsedActionMask(const Player *player);

SDL_FPoint Player_GetPosition(const Player *player);
SDL_FRect Player_GetBounds(const Player *player);
bool Player_GetMeleeHitbox(const Player *player, SDL_FRect *out_hitbox);
bool Player_ConsumeShot(Player *player, SDL_FPoint *out_origin, float *out_direction, bool *out_strong);
bool Player_IsInvulnerable(const Player *player);

#endif

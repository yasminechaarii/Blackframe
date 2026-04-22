/*
 * player.c  –  Linux / SDL2 version
 * Replaces the Windows-only WIC image loader with SDL2_image (IMG_Load).
 * All game logic is identical to the original.
 */

#include "player.h"

#include <math.h>
#include <string.h>
#include <SDL2/SDL_image.h>


#define PLAYER_MAX_ROWS            16
#define PLAYER_DETECTION_THRESHOLD 10
#define PLAYER_DETECTION_MIN_BAND  24
#define PLAYER_DETECTION_MIN_FRAME 24
#define PLAYER_FRAME_PADDING        4   // <--- Make sure this line is here
#define PLAYER_RENDER_SCALE        1.15f

typedef struct IntRange {
    int start;
    int end;
} IntRange;

static const char *k_action_names[PLAYER_ACTION_COUNT] = {
    "Walk",
    "Run",
    "Jump",
    "Dodge",
    "Kick",
    "Punch",
    "Flying Kick",
    "Shot",
    "Crouch Shot",
    "Power Shot"
};

static const float k_action_fps[PLAYER_ACTION_COUNT] = {
    8.0f, 10.0f, 11.0f, 9.0f, 10.0f, 10.0f, 9.0f, 8.0f, 8.0f, 7.0f
};

static const bool k_action_looping[PLAYER_ACTION_COUNT] = {
    true, true, false, false, false, false, false, false, false, false
};

static const float k_action_durations[PLAYER_ACTION_COUNT] = {
    0.0f, 0.0f, 0.65f, 0.70f, 0.48f, 0.46f, 0.62f, 0.50f, 0.48f, 0.60f
};

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static float clampf(float value, float minimum, float maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

static float approachf(float current, float target, float delta)
{
    if (current < target) {
        current += delta;
        if (current > target) current = target;
    } else if (current > target) {
        current -= delta;
        if (current < target) current = target;
    }
    return current;
}

/* ------------------------------------------------------------------ */
/* PNG loading via SDL2_image (replaces Windows WIC)                  */
/* ------------------------------------------------------------------ */

static SDL_Surface *load_surface_png(const char *path)
{
    /* IMG_Load returns an SDL_Surface* with the image in its native
       pixel format.  We then convert it to BGRA32 so the rest of the
       sprite-detection code (which reads B/G/R/A at offsets 0/1/2/3)
       works exactly like the original WIC implementation. */
    SDL_Surface *raw = IMG_Load(path);
    if (raw == NULL) {
        SDL_SetError("IMG_Load failed for '%s': %s", path, IMG_GetError());
        return NULL;
    }

    SDL_Surface *converted = SDL_ConvertSurfaceFormat(raw, SDL_PIXELFORMAT_BGRA32, 0);
    SDL_FreeSurface(raw);
    if (converted == NULL) {
        SDL_SetError("SDL_ConvertSurfaceFormat failed: %s", SDL_GetError());
        return NULL;
    }

    return converted;
}

/* ------------------------------------------------------------------ */
/* Sprite-sheet auto-detection (unchanged from original)              */
/* ------------------------------------------------------------------ */

static bool pixel_is_occupied(const SDL_Surface *surface, int x, int y)
{
    const Uint8 *pixel = (const Uint8 *)surface->pixels + y * surface->pitch + x * 4;
    const int b = pixel[0];
    const int g = pixel[1];
    const int r = pixel[2];
    const int a = pixel[3];
    return a > 10 && (r + g + b) > PLAYER_DETECTION_THRESHOLD;
}

static int find_row_bands(const SDL_Surface *surface, IntRange *out_bands, int max_bands)
{
    int  count     = 0;
    bool in_band   = false;
    int  band_start = 0;

    for (int y = 0; y < surface->h; ++y) {
        int occupied = 0;
        for (int x = 0; x < surface->w; ++x) {
            if (pixel_is_occupied(surface, x, y)) ++occupied;
        }

        if (occupied > 8) {
            if (!in_band) { band_start = y; in_band = true; }
        } else if (in_band) {
            if ((y - band_start) >= PLAYER_DETECTION_MIN_BAND && count < max_bands) {
                out_bands[count].start = band_start;
                out_bands[count].end   = y - 1;
                ++count;
            }
            in_band = false;
        }
    }

    if (in_band && count < max_bands &&
        (surface->h - band_start) >= PLAYER_DETECTION_MIN_BAND) {
        out_bands[count].start = band_start;
        out_bands[count].end   = surface->h - 1;
        ++count;
    }

    return count;
}

static int find_frames_in_band(const SDL_Surface *surface,
                                const IntRange    *band,
                                SDL_Rect          *out_frames,
                                int                max_frames)
{
    int  count         = 0;
    bool in_segment    = false;
    int  segment_start = 0;

    for (int x = 0; x < surface->w; ++x) {
        int occupied = 0;
        for (int y = band->start; y <= band->end; ++y) {
            if (pixel_is_occupied(surface, x, y)) ++occupied;
        }

        if (occupied > 8) {
            if (!in_segment) { segment_start = x; in_segment = true; }
        } else if (in_segment) {
            const int segment_end = x - 1;
            const int width       = segment_end - segment_start + 1;
            if (width >= PLAYER_DETECTION_MIN_FRAME && count < max_frames) {
                out_frames[count].x = SDL_max(0, segment_start - PLAYER_FRAME_PADDING);
                out_frames[count].y = SDL_max(0, band->start   - PLAYER_FRAME_PADDING);
                out_frames[count].w = SDL_min(surface->w - out_frames[count].x,
                                              width + PLAYER_FRAME_PADDING * 2);
                out_frames[count].h = SDL_min(surface->h - out_frames[count].y,
                                              (band->end - band->start + 1) + PLAYER_FRAME_PADDING * 2);
                ++count;
            }
            in_segment = false;
        }
    }

    if (in_segment && count < max_frames) {
        const int segment_end = surface->w - 1;
        const int width       = segment_end - segment_start + 1;
        if (width >= PLAYER_DETECTION_MIN_FRAME) {
            out_frames[count].x = SDL_max(0, segment_start - PLAYER_FRAME_PADDING);
            out_frames[count].y = SDL_max(0, band->start   - PLAYER_FRAME_PADDING);
            out_frames[count].w = SDL_min(surface->w - out_frames[count].x,
                                          width + PLAYER_FRAME_PADDING * 2);
            out_frames[count].h = SDL_min(surface->h - out_frames[count].y,
                                          (band->end - band->start + 1) + PLAYER_FRAME_PADDING * 2);
            ++count;
        }
    }

    return count;
}

static bool detect_animation_clips(Player *player, const SDL_Surface *surface)
{
    IntRange row_bands[PLAYER_MAX_ROWS];
    const int band_count = find_row_bands(surface, row_bands, PLAYER_MAX_ROWS);

    // Check if we found ANY rows
    if (band_count == 0) {
        SDL_SetError("No animation rows found in the sprite sheet.");
        return false;
    }

    // Print a warning instead of failing the entire Init process
    if (band_count < PLAYER_ACTION_COUNT) {
        fprintf(stderr, "Warning: Expected %d animation rows, but found %d. Missing actions will use the last available animation.\n", PLAYER_ACTION_COUNT, band_count);
    }

    for (int i = 0; i < PLAYER_ACTION_COUNT; ++i) {
        PlayerAnimationClip *clip = &player->clips[i];
        
        // If the row doesn't exist, safely fall back to the last detected row
        int row_index = (i < band_count) ? i : (band_count - 1);
        
        clip->frame_count = find_frames_in_band(surface, &row_bands[row_index],
                                                clip->frames, PLAYER_MAX_FRAMES);
        clip->fps     = k_action_fps[i];
        clip->looping = k_action_looping[i];

        if (clip->frame_count <= 0) {
            SDL_SetError("Animation row %d did not contain any usable frames.", row_index);
            return false;
        }
    }

    return true;
}

/* ------------------------------------------------------------------ */
/* Internal state helpers (unchanged from original)                   */
/* ------------------------------------------------------------------ */

static void set_action(Player *player, PlayerAction action, bool restart)
{
    if (!restart && player->action == action) return;
    player->action      = action;
    player->action_time = 0.0f;
    player->frame_index = 0;
    player->shot_emitted = false;
}

static void mark_action_used(Player *player, PlayerAction action)
{
    player->used_actions_mask |= (1u << (uint32_t)action);
}

static bool action_locks_controls(PlayerAction action)
{
    return action == PLAYER_ACTION_DODGE        ||
           action == PLAYER_ACTION_KICK         ||
           action == PLAYER_ACTION_PUNCH        ||
           action == PLAYER_ACTION_FLYING_KICK  ||
           action == PLAYER_ACTION_SHOOT        ||
           action == PLAYER_ACTION_CROUCH_SHOT  ||
           action == PLAYER_ACTION_POWER_SHOT;
}

static void sync_animation_frame(Player *player)
{
    const PlayerAnimationClip *clip = &player->clips[player->action];

    if (clip->frame_count <= 1) { player->frame_index = 0; return; }

    if ((player->action == PLAYER_ACTION_WALK || player->action == PLAYER_ACTION_RUN) &&
        player->on_ground && fabsf(player->vx) < 10.0f) {
        player->frame_index = 0;
        return;
    }

    if (clip->looping) {
        player->frame_index = (int)(player->action_time * clip->fps) % clip->frame_count;
    } else {
        int frame = (int)(player->action_time * clip->fps);
        if (frame >= clip->frame_count) frame = clip->frame_count - 1;
        if (frame < 0)                  frame = 0;
        player->frame_index = frame;
    }
}

static void start_ground_attack(Player *player, PlayerAction action)
{
    set_action(player, action, true);
    mark_action_used(player, action);
    if (action == PLAYER_ACTION_DODGE) {
        player->vx = player->facing_right ? 520.0f : -520.0f;
    } else if (action == PLAYER_ACTION_FLYING_KICK) {
        player->vx = player->facing_right ? 360.0f : -360.0f;
        player->vy = -260.0f;
        player->on_ground = false;
    } else {
        player->vx *= 0.35f;
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

bool Player_Init(Player *player, SDL_Renderer *renderer,
                 const char *sprite_sheet_path, float ground_y)
{
    if (player == NULL || renderer == NULL || sprite_sheet_path == NULL) {
        SDL_SetError("Player_Init received a null argument.");
        return false;
    }

    memset(player, 0, sizeof(*player));

    /* Initialise SDL2_image (PNG support). Safe to call multiple times. */
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        SDL_SetError("IMG_Init failed: %s", IMG_GetError());
        return false;
    }

    SDL_Surface *surface = load_surface_png(sprite_sheet_path);
    if (surface == NULL) return false;

    player->texture_width  = surface->w;
    player->texture_height = surface->h;

    if (!detect_animation_clips(player, surface)) {
        SDL_FreeSurface(surface);
        return false;
    }

    player->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (player->texture == NULL) return false;

    SDL_SetTextureBlendMode(player->texture, SDL_BLENDMODE_BLEND);

    player->x          = 220.0f;
    player->y          = ground_y;
    player->ground_y   = ground_y;
    player->on_ground  = true;
    player->facing_right = true;
    set_action(player, PLAYER_ACTION_WALK, true);
    player->used_actions_mask = 0u;
    return true;
}

void Player_Destroy(Player *player)
{
    if (player != NULL && player->texture != NULL) {
        SDL_DestroyTexture(player->texture);
        player->texture = NULL;
    }
}

void Player_Update(Player *player, const PlayerInput *input, float dt)
{
    const float gravity      = 2200.0f;
    const float walk_speed   = 210.0f;
    const float run_speed    = 360.0f;
    const float accel_ground = 1700.0f;
    const float accel_air    = 900.0f;
    const float friction     = 2200.0f;
    const int   direction    = (input->move_right ? 1 : 0) - (input->move_left ? 1 : 0);
    bool locked = action_locks_controls(player->action);

    player->action_time += dt;

    if (direction != 0 && !locked) player->facing_right = direction > 0;

    if (!locked) {
        if (input->jump_pressed && player->on_ground) {
            player->vy        = -920.0f;
            player->on_ground = false;
            set_action(player, PLAYER_ACTION_JUMP, true);
            mark_action_used(player, PLAYER_ACTION_JUMP);
        } else if (input->dodge_pressed      && player->on_ground) { start_ground_attack(player, PLAYER_ACTION_DODGE);       }
        else if (input->kick_pressed         && player->on_ground) { start_ground_attack(player, PLAYER_ACTION_KICK);        }
        else if (input->punch_pressed        && player->on_ground) { start_ground_attack(player, PLAYER_ACTION_PUNCH);       }
        else if (input->flying_kick_pressed)                       { start_ground_attack(player, PLAYER_ACTION_FLYING_KICK); }
        else if (input->shoot_pressed        && player->on_ground) { start_ground_attack(player, PLAYER_ACTION_SHOOT);       }
        else if (input->crouch_shot_pressed  && player->on_ground) { start_ground_attack(player, PLAYER_ACTION_CROUCH_SHOT); }
        else if (input->power_shot_pressed   && player->on_ground) { start_ground_attack(player, PLAYER_ACTION_POWER_SHOT);  }
    }

    locked = action_locks_controls(player->action);

    if (!locked) {
        const float target_speed = input->run ? run_speed : walk_speed;
        const float target_vx    = (float)direction * target_speed;
        const float accel        = player->on_ground ? accel_ground : accel_air;

        if (direction != 0) {
            player->vx = approachf(player->vx, target_vx, accel * dt);
            if (player->on_ground) {
                if (input->run) { set_action(player, PLAYER_ACTION_RUN,  false); mark_action_used(player, PLAYER_ACTION_RUN);  }
                else            { set_action(player, PLAYER_ACTION_WALK, false); mark_action_used(player, PLAYER_ACTION_WALK); }
            }
        } else {
            player->vx = approachf(player->vx, 0.0f, friction * dt);
            if (player->on_ground) set_action(player, PLAYER_ACTION_WALK, false);
        }
    } else {
        switch (player->action) {
        case PLAYER_ACTION_DODGE:       player->vx = player->facing_right ?  560.0f : -560.0f; break;
        case PLAYER_ACTION_FLYING_KICK: player->vx = player->facing_right ?  420.0f : -420.0f; break;
        default:                        player->vx = approachf(player->vx, 0.0f, 1200.0f * dt); break;
        }

        if (player->action_time >= k_action_durations[player->action]) {
            if (player->on_ground) set_action(player, PLAYER_ACTION_WALK, true);
            else                   set_action(player, PLAYER_ACTION_JUMP, true);
        }
    }

    if (!player->on_ground) {
        player->vy += gravity * dt;
        if (!locked) set_action(player, PLAYER_ACTION_JUMP, false);
    }

    player->x += player->vx * dt;
    player->y += player->vy * dt;

    if (player->y >= player->ground_y) {
        player->y        = player->ground_y;
        player->vy       = 0.0f;
        player->on_ground = true;
        if (!action_locks_controls(player->action) && player->action == PLAYER_ACTION_JUMP) {
            set_action(player, PLAYER_ACTION_WALK, true);
        }
    } else {
        player->on_ground = false;
    }

    player->x = clampf(player->x, 70.0f, 2530.0f);
    sync_animation_frame(player);
}

void Player_Render(const Player *player, SDL_Renderer *renderer, float camera_x)
{
    const PlayerAnimationClip *clip        = &player->clips[player->action];
    const SDL_Rect             source      = clip->frames[player->frame_index];
    const float                draw_width  = source.w * PLAYER_RENDER_SCALE;
    const float                draw_height = source.h * PLAYER_RENDER_SCALE;
    SDL_FRect dest = {
        player->x - camera_x - draw_width * 0.5f,
        player->y - draw_height,
        draw_width,
        draw_height
    };

    SDL_RendererFlip flip = player->facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_RenderCopyExF(renderer, player->texture, &source, &dest, 0.0, NULL, flip);
}

PlayerAction Player_GetAction(const Player *player) { return player->action; }

const char *Player_GetActionName(PlayerAction action)
{
    if (action < 0 || action >= PLAYER_ACTION_COUNT) return "Unknown";
    return k_action_names[action];
}

uint32_t Player_GetUsedActionMask(const Player *player) { return player->used_actions_mask; }

SDL_FPoint Player_GetPosition(const Player *player)
{
    SDL_FPoint point = { player->x, player->y };
    return point;
}

SDL_FRect Player_GetBounds(const Player *player)
{
    SDL_FRect bounds = { player->x - 26.0f, player->y - 142.0f, 52.0f, 142.0f };
    return bounds;
}

bool Player_GetMeleeHitbox(const Player *player, SDL_FRect *out_hitbox)
{
    const float direction = player->facing_right ? 1.0f : -1.0f;
    if (out_hitbox == NULL) return false;

    switch (player->action) {
    case PLAYER_ACTION_KICK:
        if (player->action_time >= 0.16f && player->action_time <= 0.36f) {
            out_hitbox->x = player->x + (direction > 0.0f ? 18.0f : -104.0f);
            out_hitbox->y = player->y - 118.0f;
            out_hitbox->w = 104.0f; out_hitbox->h = 64.0f;
            return true;
        }
        break;
    case PLAYER_ACTION_PUNCH:
        if (player->action_time >= 0.14f && player->action_time <= 0.30f) {
            out_hitbox->x = player->x + (direction > 0.0f ? 18.0f : -96.0f);
            out_hitbox->y = player->y - 130.0f;
            out_hitbox->w = 96.0f; out_hitbox->h = 52.0f;
            return true;
        }
        break;
    case PLAYER_ACTION_FLYING_KICK:
        if (player->action_time >= 0.18f && player->action_time <= 0.48f) {
            out_hitbox->x = player->x + (direction > 0.0f ? 6.0f : -122.0f);
            out_hitbox->y = player->y - 140.0f;
            out_hitbox->w = 122.0f; out_hitbox->h = 86.0f;
            return true;
        }
        break;
    default: break;
    }
    return false;
}

bool Player_ConsumeShot(Player *player, SDL_FPoint *out_origin,
                        float *out_direction, bool *out_strong)
{
    float shot_time = 0.18f;
    switch (player->action) {
    case PLAYER_ACTION_SHOOT:       shot_time = 0.18f; break;
    case PLAYER_ACTION_CROUCH_SHOT: shot_time = 0.16f; break;
    case PLAYER_ACTION_POWER_SHOT:  shot_time = 0.20f; break;
    default: return false;
    }

    if (player->shot_emitted || player->action_time < shot_time) return false;

    player->shot_emitted = true;

    if (out_direction) *out_direction = player->facing_right ? 1.0f : -1.0f;
    if (out_strong)    *out_strong    = (player->action == PLAYER_ACTION_POWER_SHOT);
    if (out_origin) {
        out_origin->x = player->x + (player->facing_right ? 44.0f : -44.0f);
        if      (player->action == PLAYER_ACTION_CROUCH_SHOT) out_origin->y = player->y - 64.0f;
        else if (player->action == PLAYER_ACTION_POWER_SHOT)  out_origin->y = player->y - 110.0f;
        else                                                   out_origin->y = player->y - 96.0f;
    }
    return true;
}

bool Player_IsInvulnerable(const Player *player)
{
    return player->action == PLAYER_ACTION_DODGE && player->action_time <= 0.55f;
}

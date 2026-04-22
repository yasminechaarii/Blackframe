#include "player.h"

#include <math.h>

static void move_x(Player* p, const Level* level, float amount) {
    int steps = (int)ceilf(fabsf(amount));
    if (steps <= 0) return;

    float step = amount / (float)steps;
    for (int i = 0; i < steps; i++) {
        SDL_FRect candidate = p->rect;
        candidate.x += step;

        
        SDL_FRect side_probe = candidate;
        side_probe.y += 3.0f;
        side_probe.h -= 6.0f;

        if (level_solid_rect_collides(level, side_probe)) {
            p->vx = 0.0f;
            return;
        }

        p->rect = candidate;
    }
}

static void move_y(Player* p, const Level* level, float amount) {
    int steps = (int)ceilf(fabsf(amount));
    if (steps <= 0) return;

    float step = amount / (float)steps;
    for (int i = 0; i < steps; i++) {
        SDL_FRect candidate = p->rect;
        candidate.y += step;

        SDL_FRect probe = candidate;
        if (step > 0.0f) {
           
            probe.x += 3.0f;
            probe.w -= 6.0f;
            probe.y = candidate.y + candidate.h - 2.0f;
            probe.h = 2.0f;
        } else {
            
            probe.x += 3.0f;
            probe.w -= 6.0f;
            probe.h = 2.0f;
        }

        if (level_solid_rect_collides(level, probe)) {
            if (step > 0.0f) p->on_ground = 1;
            p->vy = 0.0f;
            return;
        }

        p->rect = candidate;
    }
}

void player_init(Player* p, float x, float y) {
    p->rect = (SDL_FRect){x, y, 40, 40};
    p->vx = 0.0f;
    p->vy = 0.0f;
    p->on_ground = 0;
}

void player_handle_input(Player* p, const Uint8* keys) {
    const float speed = 320.0f;

    p->vx = 0.0f;
    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) p->vx = -speed;
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) p->vx = speed;

    if ((keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) && p->on_ground) {
        p->vy = -620.0f;
        p->on_ground = 0;
    }
}

void player_update(Player* p, const Level* level, float dt) {
    const float gravity = 1500.0f;

    p->vy += gravity * dt;

    move_x(p, level, p->vx * dt);

    p->on_ground = 0;
    move_y(p, level, p->vy * dt);

 
    if (!p->on_ground && p->vy >= 0.0f) {
        SDL_FRect probe = p->rect;
        probe.y += 3.0f;
        if (level_solid_rect_collides(level, probe)) {
            for (int i = 0; i < 3; i++) {
                p->rect.y += 1.0f;
                if (level_solid_rect_collides(level, p->rect)) {
                    p->rect.y -= 1.0f;
                    p->vy = 0.0f;
                    p->on_ground = 1;
                    break;
                }
            }
        }
    }

    if (p->rect.x < 0.0f) p->rect.x = 0.0f;
    if (p->rect.x + p->rect.w > WORLD_W) p->rect.x = WORLD_W - p->rect.w;

    if (p->rect.y > WORLD_H + 300.0f) {
        p->rect.x = (float)level->spawn_x;
        p->rect.y = (float)level->spawn_y;
        p->vx = 0.0f;
        p->vy = 0.0f;
        p->on_ground = 0;
    }
}

void player_draw(SDL_Renderer* renderer, const Player* p, const Camera* cam) {
    SDL_FRect s = cam_rect(cam, p->rect);
    SDL_SetRenderDrawColor(renderer, 230, 30, 30, 255);
    SDL_RenderFillRectF(renderer, &s);
}


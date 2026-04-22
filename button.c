#include "button.h"

void initButton(Button *b, int x, int y, int w, int h,
                SDL_Texture *normal,
                SDL_Texture *hover)
{
    b->rect = (SDL_Rect){x, y, w, h};
    b->normal = normal;
    b->hover = hover;
    b->isHovered = false;
}

void updateButtonHover(Button *b, int mouseX, int mouseY)
{
    b->isHovered =
        (mouseX >= b->rect.x &&
         mouseX <= b->rect.x + b->rect.w &&
         mouseY >= b->rect.y &&
         mouseY <= b->rect.y + b->rect.h);
}

void renderButton(Button *b, SDL_Renderer *renderer)
{
    SDL_Rect r = b->rect;

    if (b->isHovered) {
        r.x -= 5; r.y -= 5;
        r.w += 10; r.h += 10;
        SDL_RenderCopy(renderer, b->hover, NULL, &r);
    } else {
        SDL_RenderCopy(renderer, b->normal, NULL, &r);
    }
}

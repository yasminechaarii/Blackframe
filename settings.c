#include "settings.h"
#include <stdio.h>

// ================= TEXTURES =================
SDL_Texture *bg = NULL;
SDL_Texture *bakPanel = NULL;
SDL_Texture *volumePanel = NULL;
SDL_Texture *musicPanel = NULL;
SDL_Texture *onButton = NULL;
SDL_Texture *offButton = NULL;
SDL_Texture *decreaseBtn = NULL;
SDL_Texture *increaseBtn = NULL;
SDL_Texture *volumeBar = NULL;
SDL_Texture *volumeKnob = NULL;
SDL_Texture *backButton = NULL;
SDL_Texture *fullPanel = NULL;
SDL_Texture *fullscreenBtn = NULL;
SDL_Texture *normalBtn = NULL;

// Hover textures: displayed when the mouse cursor is over the button
SDL_Texture *decreaseBtnH = NULL;   // --.png
SDL_Texture *increaseBtnH = NULL;   // ++.png
SDL_Texture *fullscreenBtnH = NULL; // fullscreenn.png
SDL_Texture *normalBtnH = NULL;     // normall.png
SDL_Texture *backButtonH = NULL;    // backk.png

// ================= AUDIO =================
Mix_Music  *bgMusic = NULL;
Mix_Chunk  *clickSound = NULL;

// ================= STATE =================
int volume = 50;
int isMuted = 0;
int volumeBeforeMute = 50; // saves volume level before muting so we can restore it
int isFullscreen = 0;

// Hover state: 1 = cursor is over the button, 0 = not
int hoverDecrease   = 0;
int hoverIncrease   = 0;
int hoverFullscreen = 0;
int hoverNormal     = 0;
int hoverBack       = 0;

// -------------------------------------------------------
// Layout dimensions (in pixels)
// All positions are calculated relative to WINDOW_WIDTH/HEIGHT
// so they stay fixed even in fullscreen mode
// -------------------------------------------------------
#define PANEL_WIDTH         360
#define PANEL_HEIGHT        70
#define VOLUME_PANEL_WIDTH  300
#define VOLUME_PANEL_HEIGHT 100

#define MUSIC_PANEL_HEIGHT  60
#define BUTTON_ZONE_SIZE    55   // clickable area size for toggle buttons
#define BUTTON_IMAGE_SIZE   150  // display size of button images (can be larger than click zone)
#define BUTTON_OFFSET_LEFT  60   // distance from the right edge of the panel to the button
#define GAP_BELOW_BAK       10   // vertical gap between bak.b panel and Music panel

#define FULL_PANEL_HEIGHT   60
#define GAP_BELOW_MUSIC     10   // vertical gap between Music panel and full panel

// Fullscreen/Normal buttons are 20px larger than the standard toggle buttons
#define FS_BTN_ZONE_W       (BUTTON_ZONE_SIZE + 20)
#define FS_BTN_ZONE_H       (BUTTON_ZONE_SIZE + 20)
#define FS_BTN_IMG_SIZE     (BUTTON_IMAGE_SIZE + 20)
#define FS_LEFT_MARGIN      BUTTON_OFFSET_LEFT   // left button distance from panel left edge
#define FS_RIGHT_MARGIN     BUTTON_OFFSET_LEFT   // right button distance from panel right edge

#define BACK_BUTTON_SIZE    170
#define BACK_BUTTON_RIGHT   150  // distance from right edge of window
#define BACK_BUTTON_BOTTOM  100  // distance from bottom edge of window

// ================= RECTANGLES =================
SDL_Rect panelRect;
SDL_Rect volumePanelRect;
SDL_Rect musicPanelRect;
SDL_Rect toggleButtonRect;
SDL_Rect onButtonRect;
SDL_Rect offButtonRect;
SDL_Rect decreaseRect;
SDL_Rect increaseRect;
SDL_Rect volBarRect;
SDL_Rect volKnobRect;
SDL_Rect backButtonRect;
SDL_Rect fullPanelRect;
SDL_Rect fullscreenClickRect;  // clickable zone for fullscreen button (left)
SDL_Rect normalClickRect;      // clickable zone for normal button (right)
SDL_Rect fullscreenBtnRect;    // display rect for fullscreen.png image
SDL_Rect normalBtnRect;        // display rect for normal.png image

// Real dimensions of loaded images (retrieved via SDL_QueryTexture)
int onButtonWidth,       onButtonHeight;
int offButtonWidth,      offButtonHeight;
int backButtonWidth,     backButtonHeight;
int fullscreenBtnWidth,  fullscreenBtnHeight;
int normalBtnWidth,      normalBtnHeight;

// =====================================================
// HELPER: center an image inside a container rectangle
// Scales the image down proportionally if it exceeds maxSize,
// then centers it inside the container rect.
// =====================================================
static SDL_Rect centerImg(int imgW, int imgH, SDL_Rect container, int maxSize) {
    SDL_Rect result;
    if (imgW > 0 && imgH > 0) {
        float ratio = 1.0f;
        // Scale down only if image exceeds maxSize in any dimension
        if (imgW > maxSize || imgH > maxSize)
            ratio = (float)maxSize / (imgW > imgH ? imgW : imgH);
        result.w = (int)(imgW * ratio);
        result.h = (int)(imgH * ratio);
    } else {
        // Fallback if image dimensions are unknown
        result.w = maxSize;
        result.h = maxSize;
    }
    // Center inside container
    result.x = container.x + (container.w - result.w) / 2;
    result.y = container.y + (container.h - result.h) / 2;
    return result;
}

// =====================================================
// INIT RECTS
// Calculates all UI element positions.
// Everything is based on WINDOW_WIDTH/HEIGHT (not the physical
// screen resolution), so positions stay the same in fullscreen
// thanks to SDL_RenderSetLogicalSize.
// =====================================================
void initRects() {
    // ---- volume.png panel ----
    volumePanelRect.x = (WINDOW_WIDTH  - VOLUME_PANEL_WIDTH)  / 2 - 278;
    volumePanelRect.y = (WINDOW_HEIGHT - VOLUME_PANEL_HEIGHT) / 2 - 50;
    volumePanelRect.w = VOLUME_PANEL_WIDTH  + 80;
    volumePanelRect.h = VOLUME_PANEL_HEIGHT - 29;

    // ---- bak.b.png panel (contains +/- buttons) ----
    panelRect.x = (WINDOW_WIDTH  - PANEL_WIDTH)  / 2 - 250;
    panelRect.y = (WINDOW_HEIGHT - PANEL_HEIGHT) / 2 + 30;
    panelRect.w = PANEL_WIDTH + 30;
    panelRect.h = PANEL_HEIGHT;

    // ---- Music.png panel (same width as bak.b, directly below it) ----
    musicPanelRect.x = panelRect.x;
    musicPanelRect.y = panelRect.y + panelRect.h + GAP_BELOW_BAK;
    musicPanelRect.w = panelRect.w;
    musicPanelRect.h = MUSIC_PANEL_HEIGHT;

    // ---- Mute toggle button (click zone, smaller than the displayed image) ----
    // Positioned near the right side of the Music panel
    toggleButtonRect.x = musicPanelRect.x + musicPanelRect.w - BUTTON_ZONE_SIZE - BUTTON_OFFSET_LEFT;
    toggleButtonRect.y = musicPanelRect.y + (MUSIC_PANEL_HEIGHT - BUTTON_ZONE_SIZE) / 2;
    toggleButtonRect.w = BUTTON_ZONE_SIZE;
    toggleButtonRect.h = BUTTON_ZONE_SIZE;

    // ON/OFF images are centered inside the toggle click zone
    // (image is larger than the zone, so it overflows visually but click area stays small)
    onButtonRect.w  = BUTTON_IMAGE_SIZE;
    onButtonRect.h  = BUTTON_IMAGE_SIZE;
    onButtonRect.x  = toggleButtonRect.x + (toggleButtonRect.w - onButtonRect.w)  / 2;
    onButtonRect.y  = toggleButtonRect.y + (toggleButtonRect.h - onButtonRect.h)  / 2;

    offButtonRect.w = BUTTON_IMAGE_SIZE;
    offButtonRect.h = BUTTON_IMAGE_SIZE;
    offButtonRect.x = toggleButtonRect.x + (toggleButtonRect.w - offButtonRect.w) / 2;
    offButtonRect.y = toggleButtonRect.y + (toggleButtonRect.h - offButtonRect.h) / 2;

    // ---- full.png panel (same width as bak.b, directly below Music panel) ----
    fullPanelRect.x = panelRect.x;
    fullPanelRect.y = musicPanelRect.y + musicPanelRect.h + GAP_BELOW_MUSIC;
    fullPanelRect.w = panelRect.w;
    fullPanelRect.h = FULL_PANEL_HEIGHT;

    // ---- LEFT button: fullscreen.png (go to fullscreen) ----
    // Click zone anchored to the left of the full panel
    fullscreenClickRect.x = fullPanelRect.x + FS_LEFT_MARGIN;
    fullscreenClickRect.y = fullPanelRect.y + (FULL_PANEL_HEIGHT - FS_BTN_ZONE_H) / 2;
    fullscreenClickRect.w = FS_BTN_ZONE_W;
    fullscreenClickRect.h = FS_BTN_ZONE_H;
    // Image rect centered inside the click zone
    fullscreenBtnRect.w = FS_BTN_IMG_SIZE;
    fullscreenBtnRect.h = FS_BTN_IMG_SIZE;
    fullscreenBtnRect.x = fullscreenClickRect.x + (fullscreenClickRect.w - fullscreenBtnRect.w) / 2;
    fullscreenBtnRect.y = fullscreenClickRect.y + (fullscreenClickRect.h - fullscreenBtnRect.h) / 2;

    // ---- RIGHT button: normal.png (go back to windowed mode) ----
    // Click zone anchored to the right of the full panel
    normalClickRect.x = fullPanelRect.x + fullPanelRect.w - FS_RIGHT_MARGIN - FS_BTN_ZONE_W;
    normalClickRect.y = fullPanelRect.y + (FULL_PANEL_HEIGHT - FS_BTN_ZONE_H) / 2;
    normalClickRect.w = FS_BTN_ZONE_W;
    normalClickRect.h = FS_BTN_ZONE_H;
    normalBtnRect.w = FS_BTN_IMG_SIZE;
    normalBtnRect.h = FS_BTN_IMG_SIZE;
    normalBtnRect.x = normalClickRect.x + (normalClickRect.w - normalBtnRect.w) / 2;
    normalBtnRect.y = normalClickRect.y + (normalClickRect.h - normalBtnRect.h) / 2;

    // ---- Decrease / Increase buttons (left and right of bak.b panel) ----
    decreaseRect.x = panelRect.x + 15;
    decreaseRect.y = panelRect.y + (PANEL_HEIGHT - 50) / 2;
    decreaseRect.w = 158;
    decreaseRect.h = 50;

    increaseRect.x = panelRect.x + PANEL_WIDTH - 180;
    increaseRect.y = panelRect.y + (PANEL_HEIGHT - 50) / 2;
    increaseRect.w = 200;
    increaseRect.h = 50;

    // ---- Volume bar (horizontal slider track) ----
    #define VOLUME_BAR_WIDTH 200
    volBarRect.x = volumePanelRect.x + (VOLUME_PANEL_WIDTH - VOLUME_BAR_WIDTH) / 2 + 100;
    volBarRect.y = volumePanelRect.y + (VOLUME_PANEL_HEIGHT - 30) / 2 - 3;
    volBarRect.w = VOLUME_BAR_WIDTH;
    volBarRect.h = 10;

    // ---- Volume knob (draggable handle on the bar) ----
    volKnobRect.w = 28;
    volKnobRect.h = 45;
    // Vertically centered on the bar
    volKnobRect.y = volBarRect.y - (volKnobRect.h - volBarRect.h) / 2;

    // ---- Back button (always bottom-right of the logical window, not the physical screen) ----
    backButtonRect.w = BACK_BUTTON_SIZE;
    backButtonRect.h = BACK_BUTTON_SIZE;
    backButtonRect.x = WINDOW_WIDTH  - BACK_BUTTON_SIZE - BACK_BUTTON_RIGHT;
    backButtonRect.y = WINDOW_HEIGHT - BACK_BUTTON_SIZE - BACK_BUTTON_BOTTOM;
}

// =====================================================
// UPDATE KNOB POSITION
// Maps the current volume (0 to VOLUME_\MAX) to a pixel
// position along the volume bar.
// =====================================================
void updateKnobPosition() {
    if (volBarRect.w == 0) return;
    int minX = volBarRect.x;
    int maxX = volBarRect.x + volBarRect.w - volKnobRect.w;
    volKnobRect.x = minX + (volume * (maxX - minX)) / VOLUME_MAX;
    if (volKnobRect.x < minX) volKnobRect.x = minX;
    if (volKnobRect.x > maxX) volKnobRect.x = maxX;
}

// =====================================================
// TOGGLE MUTE
// Saves the current volume before muting so it can be
// restored when unmuting.
// =====================================================
void toggleMute() {
    if (isMuted) {
        // Unmute: restore saved volume
        isMuted = 0;
        volume  = volumeBeforeMute;
        Mix_VolumeMusic(volume * 128 / VOLUME_MAX);
        updateKnobPosition();
    } else {
        // Mute: save current volume, set to 0
        isMuted = 1;
        volumeBeforeMute = volume;
        volume = 0;
        Mix_VolumeMusic(0);
        updateKnobPosition();
    }
    // Play click sound twice simultaneously for louder effect
    if (clickSound) { Mix_PlayChannel(-1, clickSound, 0); Mix_PlayChannel(-1, clickSound, 0); }
}

void setMute(int mute) {
    if (mute && !isMuted) {
        isMuted = 1; volumeBeforeMute = volume; volume = 0;
        Mix_VolumeMusic(0); updateKnobPosition();
    } else if (!mute && isMuted) {
        isMuted = 0; volume = volumeBeforeMute;
        Mix_VolumeMusic(volume * 128 / VOLUME_MAX); updateKnobPosition();
    }
}

// =====================================================
// SET FULLSCREEN
// Uses SDL_RenderSetLogicalSize (set in loadAssets) to keep
// all UI positions fixed regardless of physical screen resolution.
// fs=1 -> fullscreen, fs=0 -> windowed
// =====================================================
void setFullscreen(SDL_Window *win, int fs) {
    if (fs == isFullscreen) return; // already in the requested mode, do nothing
    isFullscreen = fs;
    if (fs) {
        // SDL_WINDOW_FULLSCREEN_DESKTOP uses the current desktop resolution
        // without changing the video mode (safer and faster than true fullscreen)
        SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        SDL_SetWindowFullscreen(win, 0); // back to windowed
    }
    if (clickSound) { Mix_PlayChannel(-1, clickSound, 0); Mix_PlayChannel(-1, clickSound, 0); }
}

// =====================================================
// LOAD ASSETS
// =====================================================
void loadAssets(SDL_Renderer *r) {
    initRects();

    // SDL_RenderSetLogicalSize tells SDL to treat the render target as
    // WINDOW_WIDTH x WINDOW_HEIGHT regardless of the actual window/screen size.
    // This means all our hardcoded pixel positions stay correct in fullscreen.
    SDL_RenderSetLogicalSize(r, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Allocate 16 audio channels so multiple sounds can play simultaneously
    // (we play click sound twice at once for louder effect, so we need at least 2)
    Mix_AllocateChannels(16);

    bg          = IMG_LoadTexture(r, "bg.png");
    bakPanel    = IMG_LoadTexture(r, "bak.b.png");
    volumePanel = IMG_LoadTexture(r, "volume.png");
    musicPanel  = IMG_LoadTexture(r, "Music.png");
    onButton    = IMG_LoadTexture(r, "on.png");
    offButton   = IMG_LoadTexture(r, "off.png");
    decreaseBtn = IMG_LoadTexture(r, "Decrease.png");
    increaseBtn = IMG_LoadTexture(r, "Increase.png");
    volumeBar   = IMG_LoadTexture(r, "volume_bar.png");
    volumeKnob  = IMG_LoadTexture(r, "volume_knob.png");
    backButton  = IMG_LoadTexture(r, "back.png");
    fullPanel      = IMG_LoadTexture(r, "full.png");
    fullscreenBtn  = IMG_LoadTexture(r, "fullscreen.png");
    normalBtn      = IMG_LoadTexture(r, "normal.png");

    // Hover textures (shown when mouse is over the button)
    decreaseBtnH   = IMG_LoadTexture(r, "--.png");
    increaseBtnH   = IMG_LoadTexture(r, "++.png");
    fullscreenBtnH = IMG_LoadTexture(r, "fullscreenn.png");
    normalBtnH     = IMG_LoadTexture(r, "normall.png");
    backButtonH    = IMG_LoadTexture(r, "backk.png");

    if (!bg)            printf("Error bg.png: %s\n",           IMG_GetError());
    if (!bakPanel)      printf("Error bak.b.png: %s\n",        IMG_GetError());
    if (!volumePanel)   printf("Error volume.png: %s\n",       IMG_GetError());
    if (!musicPanel)    printf("Error Music.png: %s\n",        IMG_GetError());
    if (!onButton)      printf("Error on.png: %s\n",           IMG_GetError());
    if (!offButton)     printf("Error off.png: %s\n",          IMG_GetError());
    if (!decreaseBtn)   printf("Error Decrease.png: %s\n",     IMG_GetError());
    if (!increaseBtn)   printf("Error Increase.png: %s\n",     IMG_GetError());
    if (!volumeBar)     printf("Error volume_bar.png: %s\n",   IMG_GetError());
    if (!volumeKnob)    printf("Error volume_knob.png: %s\n",  IMG_GetError());
    if (!backButton)    printf("Error back.png: %s\n",         IMG_GetError());
    if (!fullPanel)     printf("Error full.png: %s\n",         IMG_GetError());
    if (!fullscreenBtn) printf("Error fullscreen.png: %s\n",   IMG_GetError());
    if (!normalBtn)     printf("Error normal.png: %s\n",       IMG_GetError());
    if (!decreaseBtnH)  printf("Error --.png: %s\n",          IMG_GetError());
    if (!increaseBtnH)  printf("Error ++.png: %s\n",          IMG_GetError());
    if (!fullscreenBtnH)printf("Error fullscreenn.png: %s\n", IMG_GetError());
    if (!normalBtnH)    printf("Error normall.png: %s\n",     IMG_GetError());
    if (!backButtonH)   printf("Error backk.png: %s\n",       IMG_GetError());

    // SDL_QueryTexture retrieves the real pixel dimensions of the loaded image.
    // We then use centerImg() to scale and center the image inside its click zone.
    if (onButton) {
        SDL_QueryTexture(onButton, NULL, NULL, &onButtonWidth, &onButtonHeight);
        onButtonRect = centerImg(onButtonWidth, onButtonHeight, toggleButtonRect, BUTTON_IMAGE_SIZE);
    }
    if (offButton) {
        SDL_QueryTexture(offButton, NULL, NULL, &offButtonWidth, &offButtonHeight);
        offButtonRect = centerImg(offButtonWidth, offButtonHeight, toggleButtonRect, BUTTON_IMAGE_SIZE);
    }
    if (fullscreenBtn) {
        SDL_QueryTexture(fullscreenBtn, NULL, NULL, &fullscreenBtnWidth, &fullscreenBtnHeight);
        fullscreenBtnRect = centerImg(fullscreenBtnWidth, fullscreenBtnHeight, fullscreenClickRect, FS_BTN_IMG_SIZE);
    }
    if (normalBtn) {
        SDL_QueryTexture(normalBtn, NULL, NULL, &normalBtnWidth, &normalBtnHeight);
        normalBtnRect = centerImg(normalBtnWidth, normalBtnHeight, normalClickRect, FS_BTN_IMG_SIZE);
    }
    if (backButton) {
        SDL_QueryTexture(backButton, NULL, NULL, &backButtonWidth, &backButtonHeight);
        backButtonRect = centerImg(backButtonWidth, backButtonHeight, backButtonRect, BACK_BUTTON_SIZE);
    }

    // Load background music and start playing it on loop (-1 = infinite loops)
    bgMusic = Mix_LoadMUS("background_sound.mp3");
    if (bgMusic) { Mix_PlayMusic(bgMusic, -1); Mix_VolumeMusic(volume * 128 / VOLUME_MAX); }
    else printf("Error music: %s\n", Mix_GetError());

    // Load click sound and set it to maximum chunk volume (0-128)
    clickSound = Mix_LoadWAV("click.wav");
    if (clickSound) Mix_VolumeChunk(clickSound, 128);
    if (!clickSound) printf("No click sound\n");

    updateKnobPosition();
}

// =====================================================
// HELPER: draw a button with optional hover texture
// If the cursor is over the button (isHover=1) AND a hover
// texture exists, draw the hover texture; otherwise draw normal.
// =====================================================
static void drawBtn(SDL_Renderer *r, SDL_Texture *normal, SDL_Texture *hover,
                    int isHover, SDL_Rect *rect) {
    SDL_Texture *tex = (isHover && hover) ? hover : normal;
    if (tex) SDL_RenderCopy(r, tex, NULL, rect);
}

// =====================================================
// RENDER UI
// Called every frame (~60fps). Draws all UI elements in order
// (back to front: background first, buttons last).
// =====================================================
void renderUI(SDL_Renderer *r) {
    // Background (fills the entire window)
    if (bg) SDL_RenderCopy(r, bg, NULL, NULL);
    else    SDL_RenderClear(r);

    // Volume panel background
    if (volumePanel) SDL_RenderCopy(r, volumePanel, NULL, &volumePanelRect);
    else { SDL_SetRenderDrawColor(r, 100,100,100,255); SDL_RenderFillRect(r, &volumePanelRect); }

    // Volume slider track
    if (volumeBar) SDL_RenderCopy(r, volumeBar, NULL, &volBarRect);
    else { SDL_SetRenderDrawColor(r, 80,80,80,255); SDL_RenderFillRect(r, &volBarRect); }

    // Volume knob (its X position is updated by updateKnobPosition())
    if (volumeKnob) SDL_RenderCopy(r, volumeKnob, NULL, &volKnobRect);
    else { SDL_SetRenderDrawColor(r, 200,100,0,255); SDL_RenderFillRect(r, &volKnobRect); }

    // bak.b panel (contains +/- buttons)
    if (bakPanel) SDL_RenderCopy(r, bakPanel, NULL, &panelRect);

    // +/- buttons with hover effect
    drawBtn(r, decreaseBtn, decreaseBtnH, hoverDecrease, &decreaseRect);
    drawBtn(r, increaseBtn, increaseBtnH, hoverIncrease, &increaseRect);

    // Music panel background
    if (musicPanel) SDL_RenderCopy(r, musicPanel, NULL, &musicPanelRect);
    else { SDL_SetRenderDrawColor(r, 150,100,200,255); SDL_RenderFillRect(r, &musicPanelRect); }

    // Mute toggle: show on.png when sound is active, off.png when muted
    if (!isMuted) {
        if (onButton)  SDL_RenderCopy(r, onButton,  NULL, &onButtonRect);
        else { SDL_SetRenderDrawColor(r, 0,200,0,255); SDL_RenderFillRect(r, &toggleButtonRect); }
    } else {
        if (offButton) SDL_RenderCopy(r, offButton, NULL, &offButtonRect);
        else { SDL_SetRenderDrawColor(r, 200,0,0,255); SDL_RenderFillRect(r, &toggleButtonRect); }
    }

    // Full panel background (contains fullscreen/normal buttons)
    if (fullPanel) SDL_RenderCopy(r, fullPanel, NULL, &fullPanelRect);
    else { SDL_SetRenderDrawColor(r, 100,150,200,255); SDL_RenderFillRect(r, &fullPanelRect); }

    // Fullscreen button (left) and Normal button (right), both with hover
    drawBtn(r, fullscreenBtn, fullscreenBtnH, hoverFullscreen, &fullscreenBtnRect);
    drawBtn(r, normalBtn,     normalBtnH,     hoverNormal,     &normalBtnRect);

    // Back button (bottom-right corner)
    if (backButton || backButtonH)
        drawBtn(r, backButton, backButtonH, hoverBack, &backButtonRect);
    else {
        SDL_SetRenderDrawColor(r, 100,100,200,255); SDL_RenderFillRect(r, &backButtonRect);
        SDL_SetRenderDrawColor(r, 255,255,255,255); SDL_RenderDrawRect(r, &backButtonRect);
    }
}

// =====================================================
// HELPER: returns 1 if point (x,y) is inside rect, 0 otherwise
// Used for click detection and hover detection.
// =====================================================
static int inRect(int x, int y, SDL_Rect *rect) {
    return x >= rect->x && x <= rect->x + rect->w &&
           y >= rect->y && y <= rect->y + rect->h;
}

// =====================================================
// HANDLE UI EVENTS
// Processes keyboard, mouse motion, mouse click, and mouse release.
// Called every frame with the current SDL event.
// =====================================================
void handleUI(SDL_Event *e, SDL_Window *win) {
    static int dragging    = 0; // 1 while the user is dragging the volume knob
    static int dragOffsetX = 0; // offset between cursor and knob left edge when drag started

    // --- KEYBOARD ---
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_ESCAPE:
                // Push a SDL_QUIT event so the main loop exits cleanly
                { SDL_Event qe; qe.type = SDL_QUIT; SDL_PushEvent(&qe); }
                break;
            case SDLK_b:
                // B key = same as clicking the back button
                if (clickSound) { Mix_PlayChannel(-1, clickSound, 0); Mix_PlayChannel(-1, clickSound, 0); }
                { SDL_Event qe; qe.type = SDL_QUIT; SDL_PushEvent(&qe); }
                break;
            case SDLK_m:
                toggleMute();
                break;
            case SDLK_f:
                setFullscreen(win, 1); // F = go fullscreen
                break;
            case SDLK_n:
                setFullscreen(win, 0); // N = go windowed
                break;
            case SDLK_PLUS:
            case SDLK_KP_PLUS:
            case SDLK_UP:
                if (!isMuted) {
                    volume += 10;
                    if (volume > VOLUME_MAX) volume = VOLUME_MAX;
                    // SDL_mixer volume is 0-128, our volume is 0-VOLUME_MAX, so we scale
                    Mix_VolumeMusic(volume * 128 / VOLUME_MAX);
                    updateKnobPosition();
                }
                break;
            case SDLK_MINUS:
            case SDLK_KP_MINUS:
            case SDLK_DOWN:
                if (!isMuted) {
                    volume -= 10;
                    if (volume < 0) volume = 0;
                    Mix_VolumeMusic(volume * 128 / VOLUME_MAX);
                    updateKnobPosition();
                }
                break;
        }
    }

    // --- MOUSE MOTION: update hover states + drag knob ---
    if (e->type == SDL_MOUSEMOTION) {
        int x = e->motion.x;
        int y = e->motion.y;

        // Update which button the cursor is currently over
        hoverDecrease   = inRect(x, y, &decreaseRect);
        hoverIncrease   = inRect(x, y, &increaseRect);
        hoverFullscreen = inRect(x, y, &fullscreenClickRect);
        hoverNormal     = inRect(x, y, &normalClickRect);
        hoverBack       = inRect(x, y, &backButtonRect);

        // If dragging the knob, update its position and recalculate volume
        if (dragging) {
            int minX = volBarRect.x;
            int maxX = volBarRect.x + volBarRect.w - volKnobRect.w;
            int newX = x - dragOffsetX;
            // Clamp knob within bar bounds
            if (newX < minX) newX = minX;
            if (newX > maxX) newX = maxX;
            volKnobRect.x = newX;
            // Convert knob position back to volume value (inverse of updateKnobPosition)
            if (maxX > minX) {
                volume = ((newX - minX) * VOLUME_MAX) / (maxX - minX);
                // Dragging the bar unmutes if previously muted
                if (isMuted) { isMuted = 0; volumeBeforeMute = volume; }
            }
            Mix_VolumeMusic(volume * 128 / VOLUME_MAX);
        }
    }

    // --- MOUSE BUTTON DOWN: handle all click actions ---
    if (e->type == SDL_MOUSEBUTTONDOWN) {
        int x = e->button.x;
        int y = e->button.y;

        if (inRect(x, y, &backButtonRect)) {
            if (clickSound) { Mix_PlayChannel(-1, clickSound, 0); Mix_PlayChannel(-1, clickSound, 0); }
            SDL_Event qe; qe.type = SDL_QUIT; SDL_PushEvent(&qe);
        }
        else if (inRect(x, y, &toggleButtonRect)) {
            toggleMute();
        }
        else if (inRect(x, y, &fullscreenClickRect)) {
            setFullscreen(win, 1);
        }
        else if (inRect(x, y, &normalClickRect)) {
            setFullscreen(win, 0);
        }
        // Start dragging the knob
        else if (inRect(x, y, &volKnobRect)) {
            dragging = 1;
            dragOffsetX = x - volKnobRect.x; // remember where inside the knob we clicked
            if (clickSound) { Mix_PlayChannel(-1, clickSound, 0); Mix_PlayChannel(-1, clickSound, 0); }
        }
        // Click anywhere on the bar: jump knob to that position
        else if (x >= volBarRect.x && x <= volBarRect.x + volBarRect.w &&
                 y >= volBarRect.y - 20 && y <= volBarRect.y + volBarRect.h + 20) {
            int minX = volBarRect.x;
            int maxX = volBarRect.x + volBarRect.w - volKnobRect.w;
            int newX = x - volKnobRect.w / 2; // center knob on click position
            if (newX < minX) newX = minX;
            if (newX > maxX) newX = maxX;
            volKnobRect.x = newX;
            if (maxX > minX) {
                volume = ((newX - minX) * VOLUME_MAX) / (maxX - minX);
                if (isMuted) { isMuted = 0; volumeBeforeMute = volume; }
            }
            Mix_VolumeMusic(volume * 128 / VOLUME_MAX);
            dragging = 1;
            dragOffsetX = x - volKnobRect.x;
            if (clickSound) { Mix_PlayChannel(-1, clickSound, 0); Mix_PlayChannel(-1, clickSound, 0); }
        }
        else if (inRect(x, y, &decreaseRect)) {
            volume -= 10; if (volume < 0) volume = 0;
            if (isMuted) { isMuted = 0; volumeBeforeMute = volume; }
            Mix_VolumeMusic(volume * 128 / VOLUME_MAX);
            updateKnobPosition();
            if (clickSound) { Mix_PlayChannel(-1, clickSound, 0); Mix_PlayChannel(-1, clickSound, 0); }
        }
        else if (inRect(x, y, &increaseRect)) {
            volume += 10; if (volume > VOLUME_MAX) volume = VOLUME_MAX;
            if (isMuted) { isMuted = 0; volumeBeforeMute = volume; }
            Mix_VolumeMusic(volume * 128 / VOLUME_MAX);
            updateKnobPosition();
            if (clickSound) { Mix_PlayChannel(-1, clickSound, 0); Mix_PlayChannel(-1, clickSound, 0); }
        }
    }

    // --- MOUSE BUTTON UP: stop dragging ---
    if (e->type == SDL_MOUSEBUTTONUP) dragging = 0;
}

// =====================================================
// CLEANUP
// Free all textures, audio, then SDL subsystems.
// Always check for NULL before freeing to avoid crashes.
// =====================================================
void cleanupAssets() {
    if (bg)             SDL_DestroyTexture(bg);
    if (bakPanel)       SDL_DestroyTexture(bakPanel);
    if (volumePanel)    SDL_DestroyTexture(volumePanel);
    if (musicPanel)     SDL_DestroyTexture(musicPanel);
    if (onButton)       SDL_DestroyTexture(onButton);
    if (offButton)      SDL_DestroyTexture(offButton);
    if (decreaseBtn)    SDL_DestroyTexture(decreaseBtn);
    if (increaseBtn)    SDL_DestroyTexture(increaseBtn);
    if (volumeBar)      SDL_DestroyTexture(volumeBar);
    if (volumeKnob)     SDL_DestroyTexture(volumeKnob);
    if (backButton)     SDL_DestroyTexture(backButton);
    if (fullPanel)      SDL_DestroyTexture(fullPanel);
    if (fullscreenBtn)  SDL_DestroyTexture(fullscreenBtn);
    if (normalBtn)      SDL_DestroyTexture(normalBtn);
    if (decreaseBtnH)   SDL_DestroyTexture(decreaseBtnH);
    if (increaseBtnH)   SDL_DestroyTexture(increaseBtnH);
    if (fullscreenBtnH) SDL_DestroyTexture(fullscreenBtnH);
    if (normalBtnH)     SDL_DestroyTexture(normalBtnH);
    if (backButtonH)    SDL_DestroyTexture(backButtonH);

    if (bgMusic)    Mix_FreeMusic(bgMusic);
    if (clickSound) Mix_FreeChunk(clickSound);
}

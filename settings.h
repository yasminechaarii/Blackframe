#ifndef SETTINGS_H
#define SETTINGS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

// ================= CONSTANTES =================
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define VOLUME_MAX    100

// ================= VARIABLES EXTERNES =================
extern int volume;
extern int isMuted;
extern int volumeBeforeMute;
extern int isFullscreen;

// Textures normales
extern SDL_Texture *bg;
extern SDL_Texture *bakPanel;
extern SDL_Texture *volumePanel;
extern SDL_Texture *musicPanel;
extern SDL_Texture *onButton;
extern SDL_Texture *offButton;
extern SDL_Texture *decreaseBtn;
extern SDL_Texture *increaseBtn;
extern SDL_Texture *volumeBar;
extern SDL_Texture *volumeKnob;
extern SDL_Texture *backButton;
extern SDL_Texture *fullPanel;
extern SDL_Texture *fullscreenBtn;
extern SDL_Texture *normalBtn;

// Textures hover
extern SDL_Texture *decreaseBtnH;    // Decreasee.png
extern SDL_Texture *increaseBtnH;    // Increasee.png
extern SDL_Texture *fullscreenBtnH;  // fullscreenn.png
extern SDL_Texture *normalBtnH;      // normall.png
extern SDL_Texture *backButtonH;     // backk.png

// Audio
extern Mix_Music  *bgMusic;
extern Mix_Chunk  *clickSound;

// Rectangles
extern SDL_Rect panelRect;
extern SDL_Rect volumePanelRect;
extern SDL_Rect musicPanelRect;
extern SDL_Rect toggleButtonRect;
extern SDL_Rect onButtonRect;
extern SDL_Rect offButtonRect;
extern SDL_Rect decreaseRect;
extern SDL_Rect increaseRect;
extern SDL_Rect volBarRect;
extern SDL_Rect volKnobRect;
extern SDL_Rect backButtonRect;
extern SDL_Rect fullPanelRect;
extern SDL_Rect fullscreenClickRect;
extern SDL_Rect normalClickRect;
extern SDL_Rect fullscreenBtnRect;
extern SDL_Rect normalBtnRect;

// Etat hover (1 = curseur dessus)
extern int hoverDecrease;
extern int hoverIncrease;
extern int hoverFullscreen;
extern int hoverNormal;
extern int hoverBack;

// ================= PROTOTYPES =================
void initRects(void);
void updateKnobPosition(void);
void toggleMute(void);
void setMute(int mute);
void setFullscreen(SDL_Window *win, int fs);
void loadAssets(SDL_Renderer *r);
void renderUI(SDL_Renderer *r);
void handleUI(SDL_Event *e, SDL_Window *win);
void cleanupAssets(void);

#endif // SETTINGS_H

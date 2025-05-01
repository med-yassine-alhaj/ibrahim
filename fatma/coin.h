#ifndef COIN_H
#define COIN_H

#include <SDL/SDL.h>
#include "hero.h"

#define MAX_COINS 10
#define FRAME_WIDTH 150
#define FRAME_HEIGHT 150
#define TOTAL_FRAMES 9

typedef struct {
    float x, y;
    int active;
    int currentFrame;
    Uint32 frameTimer;
    int frameDelay;
    SDL_Surface *spriteSheet;
    SDL_Rect *frames;
    int totalFrames;
    float speedX;
    float amplitude;
    float frequency;
    float initialY;
    SDL_Rect rect; // Pour les collisions
} Coin;

void InitCoin(Coin *coin, float startX, float startY);
void UpdateCoin(Coin *coin);
void RenderCoin(SDL_Surface *screen, Coin *coin);
void FreeCoin(Coin *coin);

#endif

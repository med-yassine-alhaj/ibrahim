#include "coin.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define FRAME_DELAY 100
#define SCALE_FACTOR 0.25f

extern SDL_Surface* ResizeSurface(SDL_Surface *src, float scale);
extern int activeCoins;

void InitCoin(Coin *coin, float startX, float startY) {
    if (!coin) {
        printf("ERREUR: Pointeur Coin NULL dans InitCoin\n");
        exit(EXIT_FAILURE);
    }
    coin->x = startX;
    coin->y = startY;
    coin->initialY = startY;
    coin->active = 1;
    coin->currentFrame = 0;
    coin->frameTimer = SDL_GetTicks();
    coin->frameDelay = FRAME_DELAY;
    coin->speedX = -2.0f;
    coin->amplitude = 50.0f;
    coin->frequency = 0.05f;

    SDL_Surface *img = IMG_Load("assets/coin.png");
    if (!img) {
        printf("ERREUR: Chargement coin.png\n");
        exit(EXIT_FAILURE);
    }
    SDL_Surface *scaledImg = ResizeSurface(img, SCALE_FACTOR);
    SDL_FreeSurface(img);
    if (!scaledImg) {
        printf("ERREUR: Redimensionnement de coin.png\n");
        exit(EXIT_FAILURE);
    }
    coin->spriteSheet = scaledImg;
    coin->totalFrames = TOTAL_FRAMES;
    coin->frames = malloc(sizeof(SDL_Rect) * TOTAL_FRAMES);
    if (!coin->frames) {
        printf("ERREUR: Allocation mémoire pour coin->frames\n");
        SDL_FreeSurface(scaledImg);
        exit(EXIT_FAILURE);
    }
    int frameWidth = (int)(FRAME_WIDTH * SCALE_FACTOR);
    int frameHeight = (int)(FRAME_HEIGHT * SCALE_FACTOR);
    for (int j = 0; j < TOTAL_FRAMES; j++) {
        coin->frames[j].x = j * frameWidth;
        coin->frames[j].y = 0;
        coin->frames[j].w = frameWidth;
        coin->frames[j].h = frameHeight;
    }

    coin->rect.x = (int)startX;
    coin->rect.y = (int)startY;
    coin->rect.w = 30; // Tighter collision box
    coin->rect.h = 30;
}

void UpdateCoin(Coin *coin) {
    if (!coin) {
        printf("ERREUR: Pointeur Coin NULL dans UpdateCoin\n");
        return;
    }
    if (!coin->active) return;

    coin->x += coin->speedX;
    coin->y = coin->initialY + coin->amplitude * sin(coin->frequency * coin->x);

    if (coin->x < -coin->rect.w) {
        coin->active = 0;
        activeCoins--;
    }

    Uint32 now = SDL_GetTicks();
    if (now - coin->frameTimer >= (Uint32)coin->frameDelay) {
        coin->currentFrame = (coin->currentFrame + 1) % coin->totalFrames;
        coin->frameTimer = now;
    }

    coin->rect.x = (int)coin->x;
    coin->rect.y = (int)coin->y;
}

void RenderCoin(SDL_Surface *screen, Coin *coin) {
    if (!coin) {
        printf("ERREUR: Pointeur Coin NULL dans RenderCoin\n");
        return;
    }
    if (!coin->active) return;

    SDL_Rect src = coin->frames[coin->currentFrame];
    SDL_Rect dst = {(int)coin->x, (int)coin->y, 0, 0};
    SDL_BlitSurface(coin->spriteSheet, &src, screen, &dst);
}

void FreeCoin(Coin *coin) {
    if (!coin) {
        printf("ERREUR: Pointeur Coin NULL dans FreeCoin\n");
        return;
    }
    if (coin->spriteSheet) {
        SDL_FreeSurface(coin->spriteSheet);
        coin->spriteSheet = NULL;
    }
    if (coin->frames) {
        free(coin->frames);
        coin->frames = NULL;
    }
}

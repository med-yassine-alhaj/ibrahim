#ifndef COLLISION_H
#define COLLISION_H

#include "money.h"
#include "coin.h"
#include "hero.h"
#include "enemy.h"

typedef struct {
    float x, y;
    int active;
    int currentFrame;
    Uint32 frameTimer;
    int frameDelay;
    SDL_Surface *spriteSheet;
    Animation animations[3];
    int currentAnimation;
} CollisionEffect;

int RectIntersect(SDL_Rect a, SDL_Rect b);
void InitCollisionEffect(CollisionEffect *effect);
void UpdateCollisionEffect(CollisionEffect *effect);
void RenderCollisionEffect(SDL_Surface *screen, CollisionEffect *effect);
void FreeCollisionEffect(CollisionEffect *effect);
void CheckCollisions(Hero *hero, Enemy2 *enemies, int numEnemies, CollisionEffect *effect,
                     Coin *coins, int numCoins, Money *money);

#endif

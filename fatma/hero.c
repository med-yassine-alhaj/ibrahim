#include "hero.h"
#include "pos.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 1700
#define SCREEN_HEIGHT 900
#define FRAME_DELAY 100
#define HIT_DURATION 500 // Durée minimale de l'animation HIT en ms

void InitHero(Hero *hero, int startX) {
    if (!hero) {
        printf("ERREUR: Pointeur Hero NULL dans InitHero\n");
        exit(EXIT_FAILURE);
    }
    memset(hero, 0, sizeof(Hero));
    hero->x = startX;
    hero->y = getHeroY();
    hero->direction = 1;
    hero->state = HERO_IDLE;
    hero->health = 1600;
    hero->maxHealth = 1600;
    hero->moveSpeed = 8.0f;
    hero->frameDelay = FRAME_DELAY;
    hero->isJumping = 0;
    hero->jumpVelocity = -15.0f;
    hero->gravity = 0.6f;
    hero->isAttacking = 0;
    hero->frameTimer = SDL_GetTicks();
    hero->lastUpdateTime = SDL_GetTicks();
    hero->lastHitTime = 0;
    hero->flip = 0;
    hero->animationPlaying = 0;
    hero->attackDamage = 20;

    printf("DEBUG: Initialisation Hero à (%d, %f)\n", startX, hero->y);

    // Créer un carré blanc de 400x400 pixels
    SDL_Surface *whiteSquare = SDL_CreateRGBSurface(0, 400, 400, 32, 0, 0, 0, 0);
    if (!whiteSquare) {
        printf("ERREUR: SDL_CreateRGBSurface\n");
        exit(EXIT_FAILURE);
    }
    SDL_FillRect(whiteSquare, NULL, SDL_MapRGB(whiteSquare->format, 255, 255, 255));

    for (int i = 0; i < 8; i++) {
        hero->animations[i].spriteSheet = whiteSquare;
        hero->animations[i].totalFrames = 1;
        hero->animations[i].frames = malloc(sizeof(SDL_Rect) * 1);
        if (!hero->animations[i].frames) {
            printf("ERREUR: Allocation mémoire échouée pour hero->animations[%d].frames\n", i);
            SDL_FreeSurface(whiteSquare);
            for (int j = 0; j < i; j++) {
                free(hero->animations[j].frames);
            }
            exit(EXIT_FAILURE);
        }
        hero->animations[i].frames[0].x = 0;
        hero->animations[i].frames[0].y = 0;
        hero->animations[i].frames[0].w = 400;
        hero->animations[i].frames[0].h = 400;
    }

    // Pas besoin de flippedSpriteSheet car le carré est symétrique
    hero->flippedSpriteSheet = NULL;

    hero->rect.w = 400;
    hero->rect.h = 400;
    hero->rect.x = startX;
    hero->rect.y = hero->y;

    printf("DEBUG: InitHero terminé avec succès\n");
}

void UpdateHero(Hero *hero, const Uint8 *keys) {
    if (!hero) {
        printf("ERREUR: Pointeur Hero NULL dans UpdateHero\n");
        return;
    }
    Uint32 now = SDL_GetTicks();
    Uint32 deltaTime = now - hero->lastUpdateTime;
    float speedFactor = deltaTime / 16.666f;

    if (hero->state < 0 || hero->state >= 8) {
        printf("ERREUR: hero->state hors limites: %d (max: 7)\n", hero->state);
        hero->state = HERO_IDLE;
    }

    enum HeroState previousState = hero->state;
    static Uint32 lastAttackTime = 0;

    if (hero->state == HERO_HIT && now - hero->lastHitTime < HIT_DURATION) {
        // Verrouiller l'état HIT jusqu'à ce que la durée minimale soit écoulée
        if (hero->animationPlaying && hero->currentFrame >= hero->animations[HERO_HIT].totalFrames - 1) {
            hero->currentFrame = 0; // Boucler l'animation HIT
        }
    } else if (hero->state == HERO_HIT) {
        hero->animationPlaying = 0;
        hero->state = HERO_IDLE;
        hero->currentFrame = 0;
    } else if (keys[SDLK_a] && now - lastAttackTime >= 500 && !hero->animationPlaying) {
        hero->state = HERO_ATTACK;
        hero->isAttacking = 1;
        hero->currentFrame = 0;
        hero->animationPlaying = 1;
        lastAttackTime = now;
    } else if (hero->animationPlaying) {
        if (hero->currentFrame >= hero->animations[hero->state].totalFrames - 1) {
            hero->animationPlaying = 0;
            if (hero->state == HERO_ATTACK) {
                hero->isAttacking = 0;
                hero->state = HERO_IDLE;
            } else if (hero->state == HERO_DEATH) {
                return;
            }
            hero->currentFrame = 0;
        }
    }

    if (!hero->animationPlaying && hero->state != HERO_DEATH) {
        if (hero->health <= 0) {
            hero->state = HERO_DEATH;
            hero->currentFrame = 0;
            hero->animationPlaying = 1;
        } else {
            if (keys[SDLK_j] && !hero->isJumping) {
                hero->state = HERO_JUMP;
                hero->isJumping = 1;
                hero->jumpVelocity = -15.0f;
                hero->currentFrame = 0;
            } else if (keys[SDLK_SPACE] && keys[SDLK_RIGHT]) {
                hero->state = HERO_RUN;
                hero->direction = 1;
                hero->flip = 0;
                hero->x += hero->moveSpeed * speedFactor * 1.5f;
                if (previousState != HERO_RUN) hero->currentFrame = 0;
            } else if (keys[SDLK_SPACE] && keys[SDLK_LEFT]) {
                hero->state = HERO_RUN;
                hero->direction = -1;
                hero->flip = 1;
                hero->x -= hero->moveSpeed * speedFactor * 1.5f;
                if (previousState != HERO_RUN) hero->currentFrame = 0;
            } else if (keys[SDLK_RIGHT]) {
                hero->state = HERO_WALK;
                hero->direction = 1;
                hero->flip = 0;
                hero->x += hero->moveSpeed * speedFactor;
                if (previousState != HERO_WALK) hero->currentFrame = 0;
            } else if (keys[SDLK_LEFT]) {
                hero->state = HERO_WALK;
                hero->direction = -1;
                hero->flip = 1;
                hero->x -= hero->moveSpeed * speedFactor;
                if (previousState != HERO_WALK) hero->currentFrame = 0;
            } else {
                hero->state = HERO_IDLE;
                if (previousState != HERO_IDLE) hero->currentFrame = 0;
            }
        }
    }

    if (hero->isJumping) {
        hero->y += hero->jumpVelocity * speedFactor;
        hero->jumpVelocity += hero->gravity * speedFactor;
        if (hero->y >= getHeroY()) {
            hero->y = getHeroY();
            hero->isJumping = 0;
            hero->state = HERO_IDLE;
            hero->currentFrame = 0;
        } else if (hero->jumpVelocity > 0) {
            hero->state = HERO_FALL;
            if (previousState != HERO_FALL) hero->currentFrame = 0;
        }
    }

    if (hero->x < 0) hero->x = 0;
    if (hero->x > SCREEN_WIDTH - hero->rect.w) hero->x = SCREEN_WIDTH - hero->rect.w;
    if (hero->y < 0) hero->y = 0;
    if (hero->y > SCREEN_HEIGHT - hero->rect.h) hero->y = SCREEN_HEIGHT - hero->rect.h;

    hero->rect.x = (int)hero->x;
    hero->rect.y = (int)hero->y;

    if (hero->animations[hero->state].totalFrames > 0 && now - hero->frameTimer >= hero->frameDelay) {
        hero->currentFrame++;
        if (hero->currentFrame >= hero->animations[hero->state].totalFrames) {
            hero->currentFrame = 0;
        }
        hero->frameTimer = now;
    }

    hero->lastUpdateTime = now;
}

void RenderHero(SDL_Surface *screen, Hero *hero) {
    if (!hero) {
        printf("ERREUR: Pointeur Hero NULL dans RenderHero\n");
        return;
    }
    Animation *anim = &hero->animations[hero->state];
    if (anim->totalFrames <= 0) {
        printf("ERREUR: Aucune frame pour l'état %d dans RenderHero\n", hero->state);
        return;
    }
    int frameIndex = hero->currentFrame % anim->totalFrames;
    SDL_Rect src = anim->frames[frameIndex];
    SDL_Rect dst = {hero->rect.x, hero->rect.y, 0, 0};

    if (hero->state == HERO_HIT) {
        printf("DEBUG: RenderHero HERO_HIT, frame=%d, src=(%d,%d,%d,%d), dst=(%d,%d)\n",
               frameIndex, src.x, src.y, src.w, src.h, dst.x, dst.y);
    }

    SDL_Surface *currentSheet = anim->spriteSheet;
    if (!currentSheet) {
        printf("ERREUR: SpriteSheet NULL pour état %d dans RenderHero\n", hero->state);
        return;
    }
    SDL_BlitSurface(currentSheet, &src, screen, &dst);
}

void FreeHero(Hero *hero) {
    if (!hero) {
        printf("ERREUR: Pointeur Hero NULL dans FreeHero\n");
        return;
    }
    if (hero->animations[0].spriteSheet) {
        SDL_FreeSurface(hero->animations[0].spriteSheet);
        hero->animations[0].spriteSheet = NULL;
    }
    for (int i = 0; i < 8; i++) {
        if (hero->animations[i].frames) {
            free(hero->animations[i].frames);
            hero->animations[i].frames = NULL;
        }
    }
}

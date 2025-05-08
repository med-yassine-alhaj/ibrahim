#include "collision.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1700
#define SCREEN_HEIGHT 900
#define INVINCIBILITY_DURATION 2000 // 2 secondes d'invincibilité après un coup
#define HERO_WIDTH 100
#define HERO_HEIGHT 100
#define BASE_ENEMY_DAMAGE 100 // Dégâts de base pour un ennemi de rang 1 (augmenté de 50 à 100)

extern int activeCoins;

int RectIntersect(SDL_Rect a, SDL_Rect b) {
    if (a.x + a.w <= b.x || b.x + b.w <= a.x) return 0;
    if (a.y + a.h <= b.y || b.y + b.h <= a.y) return 0;
    return 1;
}

void InitCollisionEffect(CollisionEffect *effect) {
    if (!effect) {
        printf("ERREUR: Pointeur CollisionEffect NULL dans InitCollisionEffect\n");
        exit(EXIT_FAILURE);
    }
    effect->active = 0;
    effect->spriteSheet = IMG_Load("assets/collision.png");
    if (!effect->spriteSheet) {
        printf("ERREUR: Chargement collision.png\n");
        exit(EXIT_FAILURE);
    }
    int frameWidth = 300;
    int frameHeight = 300;
    for (int row = 0; row < 3; row++) {
        effect->animations[row].spriteSheet = effect->spriteSheet;
        effect->animations[row].totalFrames = 7;
        effect->animations[row].frames = malloc(sizeof(SDL_Rect) * 7);
        if (!effect->animations[row].frames) {
            printf("ERREUR: Allocation mémoire échouée pour effect->animations[%d].frames\n", row);
            exit(EXIT_FAILURE);
        }
        for (int col = 0; col < 7; col++) {
            effect->animations[row].frames[col].x = col * frameWidth;
            effect->animations[row].frames[col].y = row * frameHeight;
            effect->animations[row].frames[col].w = frameWidth;
            effect->animations[row].frames[col].h = frameHeight;
        }
    }
    effect->frameDelay = 50;
    effect->currentAnimation = 0;
    printf("DEBUG: InitCollisionEffect terminé avec succès\n");
}

void UpdateCollisionEffect(CollisionEffect *effect) {
    if (!effect) {
        printf("ERREUR: Pointeur CollisionEffect NULL dans UpdateCollisionEffect\n");
        return;
    }
    Uint32 now = SDL_GetTicks();
    if (effect->active && now - effect->frameTimer >= effect->frameDelay) {
        effect->currentFrame++;
        if (effect->currentFrame >= effect->animations[effect->currentAnimation].totalFrames) {
            effect->active = 0;
        }
        effect->frameTimer = now;
    }
}

void RenderCollisionEffect(SDL_Surface *screen, CollisionEffect *effect) {
    if (!effect) {
        printf("ERREUR: Pointeur CollisionEffect NULL dans RenderCollisionEffect\n");
        return;
    }
    if (effect->active) {
        Animation *anim = &effect->animations[effect->currentAnimation];
        SDL_Rect src = anim->frames[effect->currentFrame];
        SDL_Rect dst = {(int)effect->x, (int)effect->y, 0, 0};
        SDL_BlitSurface(anim->spriteSheet, &src, screen, &dst);
    }
}

void FreeCollisionEffect(CollisionEffect *effect) {
    if (!effect) {
        printf("ERREUR: Pointeur CollisionEffect NULL dans FreeCollisionEffect\n");
        return;
    }
    if (effect->spriteSheet) {
        SDL_FreeSurface(effect->spriteSheet);
        effect->spriteSheet = NULL;
    }
    for (int i = 0; i < 3; i++) {
        if (effect->animations[i].frames) {
            free(effect->animations[i].frames);
            effect->animations[i].frames = NULL;
        }
    }
}

void CheckCollisions(Hero *hero, Enemy2 *enemies, int numEnemies, CollisionEffect *effect,
                     Coin *coins, int numCoins, Money *money) {
    if (!hero || !enemies || !effect || !coins || !money) {
        printf("ERREUR: Pointeur NULL dans CheckCollisions\n");
        return;
    }

    Uint32 now = SDL_GetTicks();

    // Hero-Enemy collisions
    for (int i = 0; i < numEnemies; i++) {
        if (enemies[i].active && enemies[i].health > 0) {
            SDL_Rect heroCollisionRect = {
                hero->rect.x + 10, hero->rect.y + 10,
                HERO_WIDTH - 20, HERO_HEIGHT - 20 // Reduced collision box for better gameplay
            };
            SDL_Rect enemyCollisionRect = {
                enemies[i].rect.x + 10, enemies[i].rect.y + 10,
                enemies[i].rect.w - 20, enemies[i].rect.h - 20 // 200x200 -> 180x180
            };

            printf("DEBUG: Hero rect=(%d,%d,%d,%d), Enemy %d rect=(%d,%d,%d,%d)\n",
                   heroCollisionRect.x, heroCollisionRect.y, heroCollisionRect.w, heroCollisionRect.h,
                   i, enemyCollisionRect.x, enemyCollisionRect.y, enemyCollisionRect.w, enemyCollisionRect.h);

            if (RectIntersect(heroCollisionRect, enemyCollisionRect)) {
                printf("DEBUG: Collision détectée entre héros et ennemi %d\n", i);
                if (hero->isAttacking && hero->state == HERO_ATTACK) {
                    // Augmenter les dégâts de base
                    int damage = hero->attackDamage * 4; // Quadrupler les dégâts de base
                    enemies[i].health -= damage;
                    printf("DEBUG: Ennemi %d touché, dégâts = %d, santé = %d\n", i, damage, enemies[i].health);
                    if (enemies[i].health <= 0) {
                        enemies[i].state = ENEMY2_DEATH;
                        enemies[i].animationPlaying = 1;
                    } else if (enemies[i].state != ENEMY2_HIT) {
                        enemies[i].state = ENEMY2_HIT;
                        enemies[i].animationPlaying = 1;
                        enemies[i].hurtStartTime = now;
                    }
                    enemies[i].currentFrame = 0;
                    enemies[i].frameTimer = now;
                    effect->currentAnimation = rand() % 3;
                    effect->active = 1;
                    effect->x = (hero->rect.x + enemies[i].rect.x) / 2;
                    effect->y = (hero->rect.y + enemies[i].rect.y) / 2;
                    effect->currentFrame = 0;
                    effect->frameTimer = now;
                }
                else if (enemies[i].state == ENEMY2_ATTACK && now - hero->lastHitTime >= INVINCIBILITY_DURATION) {
                    // Calculer les dégâts en fonction du rang de l'ennemi
                    int damage = BASE_ENEMY_DAMAGE * enemies[i].rank;
                    hero->health -= damage;
                    hero->lastHitTime = now;
                    printf("DEBUG: Héros touché par ennemi rang %d, dégâts = %d, santé = %d\n", 
                           enemies[i].rank, damage, hero->health);
                    if (hero->health <= 0) {
                        hero->state = HERO_DEATH;
                        hero->frame = 0;
                    } else if (hero->state != HERO_HIT) {
                        hero->state = HERO_HIT;
                        hero->frame = 0;
                    }
                    hero->frameTimer = now;
                    effect->currentAnimation = rand() % 3;
                    effect->active = 1;
                    effect->x = (hero->rect.x + enemies[i].rect.x) / 2;
                    effect->y = (hero->rect.y + enemies[i].rect.y) / 2;
                    effect->currentFrame = 0;
                    effect->frameTimer = now;
                }
            }
        }
    }

    // Hero-Coin collisions
    for (int i = 0; i < numCoins; i++) {
        if (coins[i].active) {
            SDL_Rect heroCollisionRect = {
                hero->rect.x + 10, hero->rect.y + 10,
                HERO_WIDTH - 20, HERO_HEIGHT - 20 // Reduced collision box for better gameplay
            };
            if (RectIntersect(heroCollisionRect, coins[i].rect)) {
                printf("DEBUG: Pièce %d collectée\n", i);
                coins[i].active = 0;
                activeCoins--;
                money->count++;
                UpdateMoneyText(money);
                SaveMoney(money);
            }
        }
    }
}

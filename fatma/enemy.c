#include "enemy.h"
#include "collision.h"
#include "pos.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SCREEN_WIDTH 1700
#define SCREEN_HEIGHT 900

SDL_Surface* ResizeSurface(SDL_Surface *src, float scale) {
    if (!src) {
        printf("ERREUR: Surface source NULL dans ResizeSurface\n");
        return NULL;
    }
    if (scale <= 0.f) scale = 1.f;
    int newW = (int)(src->w * scale);
    int newH = (int)(src->h * scale);
    if (newW < 1 || newH < 1) return NULL;
    SDL_Surface *dest = SDL_CreateRGBSurface(SDL_SWSURFACE, newW, newH, 32,
                                             0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!dest) {
        printf("ERREUR: Échec création surface dans ResizeSurface\n");
        return NULL;
    }
    for (int y = 0; y < newH; y++) {
        for (int x = 0; x < newW; x++) {
            int sx = (int)(x / scale);
            int sy = (int)(y / scale);
            Uint32 pixel = ((Uint32*)src->pixels)[sy * src->w + sx];
            ((Uint32*)dest->pixels)[y * newW + x] = pixel;
        }
    }
    return dest;
}

void InitEnemy2(Enemy2 *e, const char *spriteSheetPath, int frameWidth, int frameHeight,
                int *framesPerState, int numStates, int maxHealth, int enemyType, float scale,
                int leftBoundary, int rightBoundary, float moveSpeed, float attackRange, float retreatDistance, float attackDistance) {
    if (!e) {
        printf("ERREUR: Pointeur Enemy2 NULL dans InitEnemy2\n");
        exit(EXIT_FAILURE);
    }
    memset(e, 0, sizeof(Enemy2));
    e->x = SCREEN_WIDTH;
    e->y = (enemyType == 0) ? getEnemy1Y() : getEnemy2Y();
    e->enemyType = enemyType;
    e->direction = -1;
    e->state = ENEMY2_IDLE;
    e->rank = enemyType + 1; // Rang 1 pour enemy1, rang 2 pour enemy2
    // Ajuster la santé en fonction du rang
    e->maxHealth = (e->rank == 1) ? maxHealth / 2 : maxHealth * 2;
    e->health = e->maxHealth;
    e->moveSpeed = moveSpeed;
    e->leftBoundary = leftBoundary;
    e->rightBoundary = rightBoundary;
    e->moving = 1;
    e->moveDirection = -1;
    e->currentFrame = 0;
    e->frameDelay = 100;
    e->frameTimer = SDL_GetTicks();
    e->isFullyVisible = 0;
    e->animationPlaying = 0;
    e->hurtStartTime = 0;
    e->active = 1;
    e->meleeRange = 50.0f;
    e->numStates = numStates;
    e->attackRange = attackRange;
    e->retreatDistance = retreatDistance;
    e->attackTimer = 0;
    e->attackPhase = 0;
    e->attackDistance = attackDistance;

    printf("DEBUG: Initialisation Enemy2 à (%f, %f) avec %d états, santé %d, rang %d\n", e->x, e->y, numStates, e->maxHealth, e->rank);

    SDL_Surface *img = IMG_Load(spriteSheetPath);
    if (!img) {
        printf("ERREUR: IMG_Load %s\n", spriteSheetPath);
        exit(EXIT_FAILURE);
    }
    SDL_Surface *sheet = ResizeSurface(img, scale);
    SDL_FreeSurface(img);
    if (!sheet) {
        printf("ERREUR: ResizeSurface a retourné NULL pour %s\n", spriteSheetPath);
        exit(EXIT_FAILURE);
    }

    e->animations = malloc(sizeof(Animation) * numStates);
    if (!e->animations) {
        printf("ERREUR: Allocation mémoire pour e->animations\n");
        SDL_FreeSurface(sheet);
        exit(EXIT_FAILURE);
    }

    int scaledFrameWidth = frameWidth * scale;
    int scaledFrameHeight = frameHeight * scale;

    for (int state = 0; state < numStates; state++) {
        e->animations[state].spriteSheet = sheet;
        e->animations[state].totalFrames = framesPerState[state];
        e->animations[state].frames = malloc(sizeof(SDL_Rect) * framesPerState[state]);
        if (!e->animations[state].frames) {
            printf("ERREUR: Allocation mémoire pour e->animations[%d].frames\n", state);
            SDL_FreeSurface(sheet);
            for (int i = 0; i < state; i++) free(e->animations[i].frames);
            free(e->animations);
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < framesPerState[state]; j++) {
            e->animations[state].frames[j].x = j * scaledFrameWidth;
            e->animations[state].frames[j].y = state * scaledFrameHeight;
            e->animations[state].frames[j].w = scaledFrameWidth;
            e->animations[state].frames[j].h = scaledFrameHeight;
        }
    }

    e->rect.w = 200;
    e->rect.h = 200;
    e->rect.x = e->x;
    e->rect.y = e->y;

    printf("DEBUG: InitEnemy2 terminé avec succès\n");
}

void UpdateEnemy2(Enemy2 *e, float heroX) {
    if (!e) {
        printf("ERREUR: Pointeur Enemy2 NULL dans UpdateEnemy2\n");
        return;
    }
    if (!e->active) return;
    Uint32 now = SDL_GetTicks();

    if (e->state < 0 || e->state >= e->numStates) {
        printf("ERREUR: e->state hors limites: %d (max: %d)\n", e->state, e->numStates - 1);
        e->state = ENEMY2_IDLE;
    }

    int framesInState = e->animations[e->state].totalFrames;
    float distanceToHero = fabs(e->x - heroX);

    // Set direction based on hero position
    if (heroX < e->x) {
        e->direction = -1;
    } else if (heroX > e->x) {
        e->direction = 1;
    }

    if (e->animationPlaying) {
        if (e->currentFrame >= framesInState - 1) {
            e->animationPlaying = 0;
            if (e->state == ENEMY2_HIT) {
                if (now - e->hurtStartTime >= 500) {
                    e->state = ENEMY2_IDLE;
                    e->currentFrame = 0;
                }
            } else if (e->state == ENEMY2_DEATH) {
                e->active = 0;
            } else if (e->state == ENEMY2_ATTACK) {
                e->state = ENEMY2_IDLE;
                e->currentFrame = 0;
                e->attackPhase = 1;
                e->attackTimer = now;
            }
        }
    } else if (e->state != ENEMY2_DEATH) {
        if (!e->isFullyVisible) {
            e->state = ENEMY2_WALK;
            e->x += e->moveDirection * e->moveSpeed;
            if (e->x <= SCREEN_WIDTH - e->rect.w - 50) e->isFullyVisible = 1;
        } else {
            if (distanceToHero <= e->attackDistance) {
                if (e->attackPhase == 0 || e->attackPhase == 3) {
                    e->state = ENEMY2_ATTACK;
                    e->animationPlaying = 1;
                    e->currentFrame = 0;
                    if (e->attackPhase == 3) e->attackPhase = 0;
                } else if (e->attackPhase == 1) {
                    e->state = ENEMY2_IDLE;
                    if (now - e->attackTimer >= 500) { // Reduced from 1000ms to 500ms
                        e->attackPhase = 2;
                        e->attackTimer = now;
                    }
                } else if (e->attackPhase == 2) {
                    e->state = ENEMY2_WALK;
                    e->x += e->moveSpeed * e->direction; // Retreat in direction
                    if (now - e->attackTimer >= 500) {
                        e->attackPhase = 3;
                    }
                }
            } else {
                e->state = ENEMY2_WALK;
                e->x += e->moveSpeed * (heroX < e->x ? -1 : 1);
            }
        }
    }

    Uint32 effectiveDelay = (e->state == ENEMY2_DEATH) ? 70 : e->frameDelay;
    if (now - e->frameTimer >= effectiveDelay) {
        e->currentFrame++;
        if (e->currentFrame >= framesInState) {
            if (e->animationPlaying) {
                e->animationPlaying = 0;
            } else {
                e->currentFrame = 0;
            }
        }
        e->frameTimer = now;
    }

    e->rect.x = (int)e->x;
    e->rect.y = (int)e->y;
}

void RenderEnemy2(SDL_Surface *screen, Enemy2 *e) {
    if (!e) {
        printf("ERREUR: Pointeur Enemy2 NULL dans RenderEnemy2\n");
        return;
    }
    if (!e->active) return;
    if (e->state < 0 || e->state >= e->numStates) {
        printf("ERREUR: e->state invalide dans RenderEnemy2: %d (max: %d)\n", e->state, e->numStates - 1);
        return;
    }
    int frameIndex = e->currentFrame % e->animations[e->state].totalFrames;
    SDL_Rect src = e->animations[e->state].frames[frameIndex];
    SDL_Rect dst = {e->rect.x, e->rect.y, 0, 0};
    SDL_BlitSurface(e->animations[e->state].spriteSheet, &src, screen, &dst);
}

void FreeEnemy2(Enemy2 *e) {
    if (!e) {
        printf("ERREUR: Pointeur Enemy2 NULL dans FreeEnemy2\n");
        return;
    }
    
    if (e->animations) {
        // Libérer la feuille de sprites une seule fois car elle est partagée
        if (e->animations[0].spriteSheet) {
            SDL_FreeSurface(e->animations[0].spriteSheet);
            e->animations[0].spriteSheet = NULL;
            // Marquer les autres pointeurs de spriteSheet comme NULL car ils pointent vers la même surface
            for (int i = 1; i < e->numStates; i++) {
                e->animations[i].spriteSheet = NULL;
            }
        }
        
        // Libérer les frames de chaque animation
        for (int i = 0; i < e->numStates; i++) {
            if (e->animations[i].frames) {
                free(e->animations[i].frames);
                e->animations[i].frames = NULL;
            }
        }
        
        // Libérer le tableau d'animations
        free(e->animations);
        e->animations = NULL;
    }
    
    // Réinitialiser les autres valeurs
    e->health = 0;
    e->maxHealth = 0;
    e->active = 0;
    e->state = ENEMY2_IDLE;
    e->currentFrame = 0;
    e->animationPlaying = 0;
}

#ifndef ENEMY_H
#define ENEMY_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#define ENEMY2_IDLE         0
#define ENEMY2_WALK         1
#define ENEMY2_ATTACK       2
#define ENEMY2_HIT          3
#define ENEMY2_DEATH        4

typedef struct {
    SDL_Surface *spriteSheet;
    SDL_Rect *frames;
    int totalFrames;
} Animation;

typedef struct {
    float x, y;
    int direction;
    int state;
    int health;
    int maxHealth;
    float moveSpeed;
    Animation *animations;
    int numStates;
    SDL_Rect rect;
    int leftBoundary;
    int rightBoundary;
    int moving;
    int moveDirection;
    int currentFrame;
    Uint32 frameDelay;
    Uint32 frameTimer;
    int isFullyVisible;
    int animationPlaying;
    Uint32 hurtStartTime;
    int active;
    float meleeRange;      // Plage de mêlée pour l'attaque
    int enemyType;         // 0 pour enemy1, 1 pour enemy2
    float attackRange;     // Distance à laquelle l'ennemi commence à attaquer
    float retreatDistance; // Distance de recul après l'attaque
    Uint32 attackTimer;    // Timer pour gérer les pauses entre les attaques
    int attackPhase;       // Phase de l'attaque (0: idle, 1: attack, 2: retreat)
    float attackDistance;  // Distance à partir de laquelle l'ennemi attaque
} Enemy2;

SDL_Surface* ResizeSurface(SDL_Surface *src, float scale);

void InitEnemy2(Enemy2 *e, const char *spriteSheetPath, int frameWidth, int frameHeight,
                int *framesPerState, int numStates, int maxHealth, int enemyType, float scale,
                int leftBoundary, int rightBoundary, float moveSpeed, float attackRange, float retreatDistance, float attackDistance);
void UpdateEnemy2(Enemy2 *e, float heroX);
void RenderEnemy2(SDL_Surface *screen, Enemy2 *e);
void FreeEnemy2(Enemy2 *e);

#endif

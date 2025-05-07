#include "pos.h"
#include "config.h"
#include <SDL/SDL.h>

#define HERO_Y    500
#define ENEMY1_Y  500
#define ENEMY2_Y  500
#define COIN_Y    370

int getHeroY() { return HERO_Y; }
int getEnemy1Y() { return ENEMY1_Y; }
int getEnemy2Y() { return ENEMY2_Y; }
int getCoinY() { return COIN_Y; }

SDL_Rect getHeroUIPos() {
    SDL_Rect pos = {10, 10, 250, 125};
    return pos;
}

SDL_Rect getEnemyUIPos() {
    SDL_Rect pos = {SCREEN_WIDTH - 260, 10, 250, 125};
    return pos;
}

SDL_Rect getHeroHealthTextPos(SDL_Rect uiPos) {
    SDL_Rect pos = {uiPos.x + 125, uiPos.y + 65, 0, 0}; // 10px à droite, 20px en bas de l'UI
    return pos;
}

SDL_Rect getEnemyHealthTextPos(SDL_Rect uiPos) {
    SDL_Rect pos = {uiPos.x + 125, uiPos.y + 65, 0, 0}; // 10px à droite, 20px en bas de l'UI
    return pos;
}

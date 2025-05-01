#ifndef POS_H
#define POS_H

#include <SDL/SDL.h>

int getHeroY();
int getEnemy1Y();
int getEnemy2Y();
int getCoinY();
SDL_Rect getHeroUIPos();
SDL_Rect getEnemyUIPos();
SDL_Rect getHeroHealthTextPos(SDL_Rect uiPos);
SDL_Rect getEnemyHealthTextPos(SDL_Rect uiPos);

#endif

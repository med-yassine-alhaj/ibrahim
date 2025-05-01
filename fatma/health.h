#ifndef HEALTH_H
#define HEALTH_H

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "hero.h"
#include "enemy.h"

SDL_Surface* CreateHealthBarSurface(TTF_Font *font, int health, int maxHealth);
SDL_Surface* CreateHeroHealthBarSurface(TTF_Font *font, Hero *hero);

#endif

#ifndef UI_H
#define UI_H

#include <SDL/SDL.h>
#include "hero.h"
#include "enemy.h"

typedef struct {
    SDL_Surface *image;      // Image de l'UI (cadre) redimensionnée
    SDL_Rect position;       // Position et taille de l'UI sur l'écran
    SDL_Surface *healthBar;  // Surface de la barre de vie
    SDL_Rect healthBarPos;   // Position de la barre de vie dans le cadre
} UI;

void InitUI(UI *ui, const char *imagePath, SDL_Rect position, float scale, int isHero);
void UpdateUIHealthBar(UI *ui, SDL_Surface *healthBarSurface);
void RenderUI(SDL_Surface *screen, UI *ui);
void FreeUI(UI *ui);

#endif

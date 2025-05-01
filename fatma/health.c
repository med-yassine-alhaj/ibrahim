#include "health.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>

SDL_Surface* CreateHealthBarSurface(TTF_Font *font, int health, int maxHealth) {
    if (!font) return NULL;
    char buf[32];
    snprintf(buf, sizeof(buf), "%d%%", (health * 100) / maxHealth);
    SDL_Color color = {0, 0, 0, 0}; // Noir
    return TTF_RenderText_Solid(font, buf, color);
}

SDL_Surface* CreateHeroHealthBarSurface(TTF_Font *font, Hero *hero) {
    if (!font || !hero) return NULL;
    char buf[32];
    snprintf(buf, sizeof(buf), "%d%%", (hero->health * 100) / hero->maxHealth);
    SDL_Color color = {0, 0, 0, 0}; // Noir
    return TTF_RenderText_Solid(font, buf, color);
}

#include "ui.h"
#include <SDL/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include "enemy.h" // Pour ResizeSurface
#include "pos.h"   // Pour getHeroHealthTextPos et getEnemyHealthTextPos

extern SDL_Surface* ResizeSurface(SDL_Surface *src, float scale);

void InitUI(UI *ui, const char *imagePath, SDL_Rect position, float scale, int isHero) {
    if (!ui) {
        printf("ERREUR: Pointeur UI NULL dans InitUI\n");
        exit(EXIT_FAILURE);
    }
    // Ne pas charger l'image de cadre
    ui->image = NULL; // Initialiser à NULL explicitement
    ui->position = position;
    ui->healthBar = NULL;
    // Utiliser les positions définies dans pos.c
    if (isHero) {
        ui->healthBarPos = getHeroHealthTextPos(position);
    } else {
        ui->healthBarPos = getEnemyHealthTextPos(position);
    }
}

void UpdateUIHealthBar(UI *ui, SDL_Surface *healthBarSurface) {
    if (!ui) {
        printf("ERREUR: Pointeur UI NULL dans UpdateUIHealthBar\n");
        return;
    }
    if (ui->healthBar) {
        SDL_FreeSurface(ui->healthBar);
    }
    ui->healthBar = healthBarSurface;
}

void RenderUI(SDL_Surface *screen, UI *ui) {
    if (!screen || !ui) {
        printf("ERREUR: Pointeur NULL dans RenderUI\n");
        return;
    }
    // Ne pas afficher l'image de cadre
    // SDL_BlitSurface(ui->image, NULL, screen, &ui->position);
    if (ui->healthBar) {
        SDL_BlitSurface(ui->healthBar, NULL, screen, &ui->healthBarPos);
    }
}

void FreeUI(UI *ui) {
    if (!ui) {
        printf("ERREUR: Pointeur UI NULL dans FreeUI\n");
        return;
    }
    // Pas besoin de libérer ui->image car il est NULL
    if (ui->healthBar) {
        SDL_FreeSurface(ui->healthBar);
        ui->healthBar = NULL;
    }
}

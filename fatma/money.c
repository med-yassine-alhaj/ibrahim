#include "config.h"
#include "money.h"
#include <stdio.h>
#include <stdlib.h>

#define MONEY_FILE "money.txt"

void InitMoney(Money *money, TTF_Font *font) {
    if (!money || !font) {
        printf("ERREUR: Pointeur NULL dans InitMoney\n");
        return;
    }
    money->count = 0;
    money->font = font;
    money->textSurface = NULL; // Explicitly initialize to NULL
    money->position.x = SCREEN_WIDTH - 150; // Top-right corner
    money->position.y = 10;
    LoadMoney(money);
    UpdateMoneyText(money);
}

void UpdateMoneyText(Money *money) {
    if (!money || !money->font) {
        printf("ERREUR: Pointeur NULL dans UpdateMoneyText\n");
        return;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "Coins: %d", money->count);
    SDL_Color color = {255, 255, 255, 0}; // White
    if (money->textSurface) {
        SDL_FreeSurface(money->textSurface);
        money->textSurface = NULL;
    }
    money->textSurface = TTF_RenderText_Solid(money->font, buf, color);
    if (!money->textSurface) {
        printf("ERREUR: Échec de TTF_RenderText_Solid dans UpdateMoneyText\n");
    }
}

void RenderMoney(SDL_Surface *screen, Money *money) {
    if (!screen || !money) {
        printf("ERREUR: Pointeur NULL dans RenderMoney\n");
        return;
    }
    if (money->textSurface) {
        SDL_BlitSurface(money->textSurface, NULL, screen, &money->position);
    }
}

void SaveMoney(Money *money) {
    if (!money) {
        printf("ERREUR: Pointeur NULL dans SaveMoney\n");
        return;
    }
    FILE *file = fopen(MONEY_FILE, "w");
    if (file) {
        fprintf(file, "%d\n", money->count);
        fclose(file);
    } else {
        printf("ERREUR: Impossible d'ouvrir %s pour écriture\n", MONEY_FILE);
    }
}

void LoadMoney(Money *money) {
    if (!money) {
        printf("ERREUR: Pointeur NULL dans LoadMoney\n");
        return;
    }
    FILE *file = fopen(MONEY_FILE, "r");
    if (file) {
        if (fscanf(file, "%d", &money->count) != 1) {
            printf("ERREUR: Échec de lecture dans %s\n", MONEY_FILE);
            money->count = 0; // Reset to default on read failure
        }
        fclose(file);
    } else {
        printf("AVERTISSEMENT: %s n'existe pas, count reste à %d\n", MONEY_FILE, money->count);
    }
}

void FreeMoney(Money *money) {
    if (!money) {
        printf("ERREUR: Pointeur NULL dans FreeMoney\n");
        return;
    }
    if (money->textSurface) {
        SDL_FreeSurface(money->textSurface);
        money->textSurface = NULL;
    }
}

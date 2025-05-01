#ifndef MONEY_H
#define MONEY_H

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

typedef struct {
    int count;
    TTF_Font *font;
    SDL_Surface *textSurface;
    SDL_Rect position;
} Money;

void InitMoney(Money *money, TTF_Font *font);
void UpdateMoneyText(Money *money);
void RenderMoney(SDL_Surface *screen, Money *money);
void SaveMoney(Money *money);
void LoadMoney(Money *money);
void FreeMoney(Money *money);

#endif

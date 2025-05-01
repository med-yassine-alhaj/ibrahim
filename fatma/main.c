#include "config.h"
#include "enemy.h"
#include "hero.h"
#include "collision.h"
#include "health.h"
#include "pos.h"
#include "coin.h"
#include "money.h"
#include "ui.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FRAME_TIME (1000/60)
#define MAX_ENEMIES 2
#define COIN_SPAWN_INTERVAL 1000

int activeCoins;

int main(void) {
    srand(time(NULL));
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) {
        printf("ERREUR: Initialisation SDL/TTF\n");
        return 1;
    }
    SDL_Surface *screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
    if (!screen) {
        printf("ERREUR: SDL_SetVideoMode\n");
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Surface *bg = IMG_Load("assets/bg.jpg");
    TTF_Font *font = TTF_OpenFont("assets/arial.ttf", 24);
    if (!bg || !font) {
        printf("ERREUR: Chargement bg.jpg ou arial.ttf\n");
        SDL_Quit();
        TTF_Quit();
        return 1;
    }

    Hero hero;
    InitHero(&hero, 100);

    Enemy2 enemies[MAX_ENEMIES];
    int eFramesPerState[] = {10, 10, 10, 10, 10};
    InitEnemy2(&enemies[0], "assets/enemy1.png", 300, 300, eFramesPerState, 5, 1600, 0, 1.0f, 0, SCREEN_WIDTH, 4.0f, 100.0f, 50.0f, 150.0f);
    InitEnemy2(&enemies[1], "assets/enemy2.png", 300, 300, eFramesPerState, 5, 1400, 1, 1.0f, 0, SCREEN_WIDTH, 4.0f, 100.0f, 50.0f, 150.0f);
    enemies[1].active = 0;

    CollisionEffect collisionEffect;
    InitCollisionEffect(&collisionEffect);

    Coin coins[MAX_COINS];
    for (int i = 0; i < MAX_COINS; i++) {
        InitCoin(&coins[i], SCREEN_WIDTH + i * 200, getCoinY());
        coins[i].active = 0;
    }
    Uint32 lastCoinSpawn = 0;
    activeCoins = 0;

    Money money;
    InitMoney(&money, font);

    UI heroUI;
    InitUI(&heroUI, "assets/ui/heroui.png", getHeroUIPos(), 0.25f, 1);
    UI enemy1UI;
    InitUI(&enemy1UI, "assets/ui/enemy1ui.png", getEnemyUIPos(), 0.25f, 0);
    UI enemy2UI;
    InitUI(&enemy2UI, "assets/ui/enemy2ui.png", getEnemyUIPos(), 0.25f, 0);

    int running = 1;
    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT || (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)) {
                running = 0;
            }
        }

        const Uint8 *keys = SDL_GetKeyState(NULL);
        UpdateHero(&hero, keys);

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                UpdateEnemy2(&enemies[i], hero.x);
            }
        }

        if (!enemies[0].active && !enemies[1].active) {
            float randVal = (float)rand() / RAND_MAX;
            int enemyToActivate = (randVal < 0.7) ? 0 : 1;
            enemies[enemyToActivate].active = 1;
            enemies[enemyToActivate].x = 1600;
            enemies[enemyToActivate].y = (enemies[enemyToActivate].enemyType == 0) ? getEnemy1Y() : getEnemy2Y();
            enemies[enemyToActivate].direction = -1;
            enemies[enemyToActivate].state = ENEMY2_IDLE;
            enemies[enemyToActivate].health = enemies[enemyToActivate].maxHealth;
            enemies[enemyToActivate].currentFrame = 0;
            enemies[enemyToActivate].frameTimer = SDL_GetTicks();
            enemies[enemyToActivate].isFullyVisible = 0;
            enemies[enemyToActivate].animationPlaying = 0;
            enemies[enemyToActivate].hurtStartTime = 0;
            enemies[enemyToActivate].attackTimer = 0;
            enemies[enemyToActivate].attackPhase = 0;
        }

        Uint32 now = SDL_GetTicks();
        if (now - lastCoinSpawn >= COIN_SPAWN_INTERVAL && activeCoins < 5) {
            for (int i = 0; i < MAX_COINS; i++) {
                if (!coins[i].active) {
                    InitCoin(&coins[i], SCREEN_WIDTH, getCoinY());
                    coins[i].active = 1;
                    activeCoins++;
                    lastCoinSpawn = now;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_COINS; i++) {
            UpdateCoin(&coins[i]);
        }

        CheckCollisions(&hero, enemies, MAX_ENEMIES, &collisionEffect, coins, MAX_COINS, &money);
        UpdateCollisionEffect(&collisionEffect);

        SDL_BlitSurface(bg, NULL, screen, NULL);
        RenderHero(screen, &hero);
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                RenderEnemy2(screen, &enemies[i]);
            }
        }
        for (int i = 0; i < MAX_COINS; i++) {
            RenderCoin(screen, &coins[i]);
        }
        RenderCollisionEffect(screen, &collisionEffect);

        UpdateUIHealthBar(&heroUI, CreateHeroHealthBarSurface(font, &hero));
        RenderUI(screen, &heroUI);

        int activeEnemyIndex = -1;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                activeEnemyIndex = i;
                break;
            }
        }
        if (activeEnemyIndex != -1) {
            UI *currentEnemyUI = (enemies[activeEnemyIndex].enemyType == 0) ? &enemy1UI : &enemy2UI;
            UpdateUIHealthBar(currentEnemyUI, CreateHealthBarSurface(font, enemies[activeEnemyIndex].health, enemies[activeEnemyIndex].maxHealth));
            RenderUI(screen, currentEnemyUI);
        }

        RenderMoney(screen, &money);

        SDL_Flip(screen);

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_TIME) SDL_Delay(FRAME_TIME - frameTime);
    }

    FreeHero(&hero);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        FreeEnemy2(&enemies[i]);
    }
    FreeCollisionEffect(&collisionEffect);
    for (int i = 0; i < MAX_COINS; i++) {
        FreeCoin(&coins[i]);
    }
    FreeMoney(&money);
    FreeUI(&heroUI);
    FreeUI(&enemy1UI);
    FreeUI(&enemy2UI);

    TTF_CloseFont(font);
    SDL_FreeSurface(bg);
    SDL_FreeSurface(screen);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

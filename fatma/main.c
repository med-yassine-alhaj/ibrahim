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
        return 1;
    }

    // Charger les bannières
    SDL_Surface* winBanner = IMG_Load("assets/ui/win.png");
    SDL_Surface* loseBanner = IMG_Load("assets/ui/lose.png");
    if (!winBanner || !loseBanner) {
        printf("Erreur lors du chargement des bannières : %s\n", IMG_GetError());
        // Créer des bannières par défaut si les images ne sont pas trouvées
        winBanner = SDL_CreateRGBSurface(0, 800, 200, 32, 0, 0, 0, 0);
        loseBanner = SDL_CreateRGBSurface(0, 800, 200, 32, 0, 0, 0, 0);
        SDL_FillRect(winBanner, NULL, SDL_MapRGB(winBanner->format, 0, 255, 0));
        SDL_FillRect(loseBanner, NULL, SDL_MapRGB(loseBanner->format, 255, 0, 0));
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
    int gameOver = 0; // 0: en cours, 1: perdu, 2: gagné
    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT || (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)) {
                running = 0;
            }
        }

        if (!gameOver) {
            const Uint8 *keys = SDL_GetKeyState(NULL);
            UpdateHero(&hero, keys);

            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    UpdateEnemy2(&enemies[i], hero.x);
                }
            }

            // Vérifier si le héros est mort
            if (hero.health <= 0) {
                gameOver = 1; // Défaite
            }
            
            // Vérifier l'état des ennemis
            int firstEnemyDead = (enemies[0].state == ENEMY2_DEATH);
            int secondEnemyDead = (enemies[1].state == ENEMY2_DEATH);
            
            // Si le premier ennemi est mort et le deuxième n'est pas encore actif
            if (firstEnemyDead && !enemies[1].active) {
                // Activer le deuxième ennemi
                enemies[1].active = 1;
                enemies[1].state = ENEMY2_IDLE;
                enemies[1].health = 2000; // Santé doublée pour le deuxième ennemi
                enemies[1].maxHealth = 2000;
                enemies[1].rank = 2; // Rang 2
                enemies[1].rect.x = 1042;
                enemies[1].rect.y = 460;
                enemies[1].rect.w = 180;
                enemies[1].rect.h = 180;
                enemies[1].direction = 1;
                enemies[1].currentFrame = 0;
                enemies[1].frameTimer = SDL_GetTicks();
                enemies[1].hurtStartTime = 0;
                enemies[1].animationPlaying = 0;
            }
            
            // Si les deux ennemis sont morts, victoire
            if (firstEnemyDead && secondEnemyDead) {
                gameOver = 2; // Victoire
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
        }

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

            // Afficher le rang de l'ennemi
            char rankText[32];
            snprintf(rankText, sizeof(rankText), "RANG %d", enemies[activeEnemyIndex].rank);
            SDL_Color textColor = {255, 255, 255, 255}; // Blanc
            SDL_Surface* textSurface = TTF_RenderText_Blended(font, rankText, textColor);
            if (textSurface) {
                SDL_Rect textRect = {
                    SCREEN_WIDTH / 2 - textSurface->w / 2,
                    20, // Position en haut de l'écran
                    textSurface->w,
                    textSurface->h
                };
                SDL_BlitSurface(textSurface, NULL, screen, &textRect);
                SDL_FreeSurface(textSurface);
            }
        }

        RenderMoney(screen, &money);

        // Afficher la bannière appropriée si le jeu est terminé
        if (gameOver) {
            SDL_Surface* banner = (gameOver == 2) ? winBanner : loseBanner;
            SDL_Rect bannerRect = {
                SCREEN_WIDTH / 2 - banner->w / 2,
                SCREEN_HEIGHT / 2 - banner->h / 2,
                banner->w,
                banner->h
            };
            SDL_BlitSurface(banner, NULL, screen, &bannerRect);
            SDL_Flip(screen);
            SDL_Delay(3000); // Afficher la bannière pendant 3 secondes
            break; // Quitter la boucle principale
        }

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
    SDL_FreeSurface(winBanner);
    SDL_FreeSurface(loseBanner);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

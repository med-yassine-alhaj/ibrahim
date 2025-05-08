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
    SDL_Surface *screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) {
        printf("ERREUR: SDL_SetVideoMode\n");
        return 1;
    }

    // Charger le fond d'écran
    SDL_Surface* bg = IMG_Load("assets/bg.jpg");
    SDL_Surface* bg2 = IMG_Load("assets/back2.jpeg");
    if (!bg || !bg2) {
        printf("Erreur de chargement du fond d'écran: %s\n", IMG_GetError());
        return -1;
    }
    TTF_Font *font = TTF_OpenFont("assets/arial.ttf", 20); // Increased font size for banners
    if (!font) {
        printf("ERREUR: Chargement arial.ttf\n");
        SDL_Quit();
        TTF_Quit();
        return 1;
    }

    // Créer les bannières avec une taille plus grande
    SDL_Surface* winBanner = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA, 800, 200, 32, 0, 0, 0, 0);
    SDL_Surface* loseBanner = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA, 800, 200, 32, 0, 0, 0, 0);
    
    // Remplir les bannières avec des couleurs semi-transparentes
    SDL_FillRect(winBanner, NULL, SDL_MapRGBA(winBanner->format, 0, 255, 0, 192));
    SDL_FillRect(loseBanner, NULL, SDL_MapRGBA(loseBanner->format, 255, 0, 0, 192));

    // Ajouter du texte aux bannières
    SDL_Color textColor = {255, 255, 255, 255}; // Blanc
    SDL_Surface* winText = TTF_RenderText_Blended(font, "VICTOIRE!", textColor);
    SDL_Surface* loseText = TTF_RenderText_Blended(font, "DEFAITE!", textColor);
    
    if (winText && loseText) {
        SDL_Rect winTextRect = {
            (winBanner->w - winText->w) / 2,
            (winBanner->h - winText->h) / 2,
            winText->w,
            winText->h
        };
        SDL_Rect loseTextRect = {
            (loseBanner->w - loseText->w) / 2,
            (loseBanner->h - loseText->h) / 2,
            loseText->w,
            loseText->h
        };
        
        SDL_BlitSurface(winText, NULL, winBanner, &winTextRect);
        SDL_BlitSurface(loseText, NULL, loseBanner, &loseTextRect);
        
        SDL_FreeSurface(winText);
        SDL_FreeSurface(loseText);
    }

    Hero hero;
    int level = 1; // Ajout d'une variable pour suivre le niveau actuel
    InitHero(&hero, 100);

    Enemy2 enemies[MAX_ENEMIES];
    int eFramesPerState[] = {10, 10, 10, 10, 10};
    InitEnemy2(&enemies[0], "assets/enemy1.png", 300, 300, eFramesPerState, 5, 3200, 0, 1.0f, 0, SCREEN_WIDTH, 4.0f, 100.0f, 50.0f, 150.0f);
    InitEnemy2(&enemies[1], "assets/enemy2.png", 300, 300, eFramesPerState, 5, 2800, 1, 1.0f, 0, SCREEN_WIDTH, 4.0f, 100.0f, 50.0f, 150.0f);
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
    Uint32 gameOverStartTime = 0;
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
                hero.health = 0; // Ensure health doesn't go below 0
                hero.state = HERO_DEATH;
                gameOver = 1; // Défaite
                gameOverStartTime = SDL_GetTicks();
            }
            
            // Vérifier l'état des ennemis
            int firstEnemyDead = (enemies[0].state == ENEMY2_DEATH);
            int secondEnemyDead = (enemies[1].state == ENEMY2_DEATH);
            
            // Si le premier ennemi est mort et le deuxième n'est pas encore actif
            if (firstEnemyDead && !enemies[1].active) {
                // Passer au niveau 2
                level = 2;
                
                // Libérer les anciens sprites du héros avant de le réinitialiser
                FreeHero(&hero);
                
                // Réinitialiser le héros avec les nouveaux sprites
                InitHero(&hero, 2); // 2 indique d'utiliser les sprites du héros 2
                
                // Activer l'ennemi 2
                enemies[1].active = 1;
            }
            
            // Si les deux ennemis sont morts, victoire
            if (firstEnemyDead && secondEnemyDead) {
                gameOver = 2; // Victoire
                gameOverStartTime = SDL_GetTicks();
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

        // Afficher le fond d'écran approprié
        SDL_BlitSurface((level == 2) ? bg2 : bg, NULL, screen, NULL);
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
            // Créer une surface semi-transparente pour l'overlay
            SDL_Surface* overlay = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA, 
                                                      SCREEN_WIDTH, SCREEN_HEIGHT, 32, 
                                                      0, 0, 0, 0);
            SDL_FillRect(overlay, NULL, SDL_MapRGBA(overlay->format, 0, 0, 0, 128));
            
            // Afficher l'overlay
            SDL_BlitSurface(overlay, NULL, screen, NULL);
            SDL_FreeSurface(overlay);
            
            // Afficher la bannière appropriée
            SDL_Surface* currentBanner = (gameOver == 2) ? winBanner : loseBanner;
            SDL_Rect bannerPos = {
                (SCREEN_WIDTH - currentBanner->w) / 2,
                (SCREEN_HEIGHT - currentBanner->h) / 2,
                currentBanner->w,
                currentBanner->h
            };
            SDL_BlitSurface(currentBanner, NULL, screen, &bannerPos);

            // Vérifier si 3 secondes se sont écoulées
            if (SDL_GetTicks() - gameOverStartTime >= 3000) {
                running = 0;
            }
        }

        SDL_Flip(screen);

        // Limiter le framerate à environ 60 FPS
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < 16) {  // 1000ms/60fps ≈ 16.67ms
            SDL_Delay(16 - frameTime);
        }
    }

    // Nettoyage
    SDL_FreeSurface(winBanner);
    SDL_FreeSurface(loseBanner);
    SDL_FreeSurface(bg);
    SDL_FreeSurface(bg2);
    TTF_CloseFont(font);
    
    // Libérer les ressources des UI
    FreeUI(&heroUI);
    FreeUI(&enemy1UI);
    FreeUI(&enemy2UI);
    
    // Libérer les ressources du héros et des ennemis
    FreeHero(&hero);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            FreeEnemy2(&enemies[i]);
        }
    }
    
    // Libérer les ressources des pièces
    for (int i = 0; i < MAX_COINS; i++) {
        if (coins[i].active) {
            FreeCoin(&coins[i]);
        }
    }

    TTF_Quit();
    SDL_Quit();
    return 0;
}


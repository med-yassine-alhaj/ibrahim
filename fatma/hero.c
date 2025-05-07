#include "hero.h"
#include "pos.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 1700
#define SCREEN_HEIGHT 900
#define FRAME_DELAY 100
#define HIT_DURATION 500
#define HERO_WIDTH 100
#define HERO_HEIGHT 100

void InitHero(Hero *hero, int startX) {
    if (!hero) {
        printf("ERREUR: Pointeur Hero NULL dans InitHero\n");
        exit(EXIT_FAILURE);
    }
    memset(hero, 0, sizeof(Hero));
    
    // Initialisation des positions et états
    hero->x = startX;
    hero->y = getHeroY();
    hero->direction = 1;
    hero->state = HERO_IDLE;
    hero->health = 1600;
    hero->maxHealth = 1600;
    hero->moveSpeed = 8.0f;
    hero->acceleration = 5.0f;
    hero->frameDelay = FRAME_DELAY;
    hero->isJumping = 0;
    hero->jumpVelocity = -15.0f;
    hero->gravity = 0.6f;
    hero->isAttacking = 0;
    hero->frameTimer = SDL_GetTicks();
    hero->lastUpdateTime = SDL_GetTicks();
    hero->lastHitTime = 0;
    hero->attackDamage = 20;
    hero->score = 0;

    // Chargement des sprites du héros
    char filename[30];
    for (int i = 0; i < 6; i++) {
        sprintf(filename, "fatma/assets/hero/perso%d.png", i + 11);
        hero->img_per[i] = IMG_Load(filename);
        if (hero->img_per[i] == NULL) {
            printf("Erreur de chargement %s : %s\n", filename, SDL_GetError());
            exit(EXIT_FAILURE);
        }
    }

    // Initialisation de la barre de vie
    hero->img_vie = IMG_Load("fatma/assets/hero/vie.png");
    if (hero->img_vie == NULL) {
        printf("Erreur chargement image vie : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    hero->pos_vie_affiche.x = 0;
    hero->pos_vie_affiche.y = 0;
    hero->pos_vie_affiche.h = 50;
    hero->pos_vie_affiche.w = 180;
    hero->pos_vie_ecran.x = 10;
    hero->pos_vie_ecran.y = 10;

    // Initialisation du texte du score
    hero->tscore.police = TTF_OpenFont("fatma/assets/hero/HIROMISAKE.ttf", 35);
    if (!hero->tscore.police) {
        printf("Erreur chargement police : %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    hero->tscore.color_txt.r = 255;
    hero->tscore.color_txt.g = 0;
    hero->tscore.color_txt.b = 0;
    hero->tscore.pos_txt.y = 10;

    // Initialisation du rectangle de collision
    hero->rect.w = HERO_WIDTH;
    hero->rect.h = HERO_HEIGHT;
    hero->rect.x = startX;
    hero->rect.y = hero->y;

    printf("DEBUG: InitHero terminé avec succès\n");
}

void UpdateHero(Hero *hero, const Uint8 *keys) {
    if (!hero) {
        printf("ERREUR: Pointeur Hero NULL dans UpdateHero\n");
        return;
    }
    Uint32 now = SDL_GetTicks();
    Uint32 deltaTime = now - hero->lastUpdateTime;
    float speedFactor = deltaTime / 16.666f;

    if (hero->state < 0 || hero->state >= 8) {
        printf("ERREUR: hero->state hors limites: %d (max: 7)\n", hero->state);
        hero->state = HERO_IDLE;
    }

    enum HeroState previousState = hero->state;
    static Uint32 lastAttackTime = 0;

    // Gestion de l'état HIT
    if (hero->state == HERO_HIT && now - hero->lastHitTime < HIT_DURATION) {
        if (hero->frame >= 5) hero->frame = 0;
    } else if (hero->state == HERO_HIT) {
        hero->state = HERO_IDLE;
        hero->frame = 0;
    }
    // Gestion de l'attaque
    else if (keys[SDLK_a] && now - lastAttackTime >= 500) {
        hero->state = HERO_ATTACK;
        hero->isAttacking = 1;
        hero->frame = 0;
        lastAttackTime = now;
    }
    // Gestion de la mort
    else if (hero->health <= 0) {
        hero->state = HERO_DEATH;
        hero->frame = 0;
    }
    // Gestion du mouvement normal
    else {
        if (keys[SDLK_j] && !hero->isJumping) {
            hero->state = HERO_JUMP;
            hero->isJumping = 1;
            hero->jumpVelocity = -15.0f;
            hero->frame = 0;
        } else if (keys[SDLK_SPACE] && keys[SDLK_RIGHT]) {
            hero->state = HERO_RUN;
            hero->direction = 1;
            hero->x += hero->moveSpeed * speedFactor * 1.5f;
            if (previousState != HERO_RUN) hero->frame = 0;
        } else if (keys[SDLK_SPACE] && keys[SDLK_LEFT]) {
            hero->state = HERO_RUN;
            hero->direction = -1;
            hero->x -= hero->moveSpeed * speedFactor * 1.5f;
            if (previousState != HERO_RUN) hero->frame = 0;
        } else if (keys[SDLK_RIGHT]) {
            hero->state = HERO_WALK;
            hero->direction = 1;
            hero->x += hero->moveSpeed * speedFactor;
            if (previousState != HERO_WALK) hero->frame = 0;
        } else if (keys[SDLK_LEFT]) {
            hero->state = HERO_WALK;
            hero->direction = -1;
            hero->x -= hero->moveSpeed * speedFactor;
            if (previousState != HERO_WALK) hero->frame = 0;
        } else {
            hero->state = HERO_IDLE;
            if (previousState != HERO_IDLE) hero->frame = 0;
        }
    }

    // Gestion du saut
    if (hero->isJumping) {
        hero->y += hero->jumpVelocity * speedFactor;
        hero->jumpVelocity += hero->gravity * speedFactor;
        if (hero->y >= getHeroY()) {
            hero->y = getHeroY();
            hero->isJumping = 0;
            hero->state = HERO_IDLE;
            hero->frame = 0;
        } else if (hero->jumpVelocity > 0) {
            hero->state = HERO_FALL;
            if (previousState != HERO_FALL) hero->frame = 0;
        }
    }

    // Limites de l'écran
    if (hero->x < 0) hero->x = 0;
    if (hero->x > SCREEN_WIDTH - hero->rect.w) hero->x = SCREEN_WIDTH - hero->rect.w;
    if (hero->y < 0) hero->y = 0;
    if (hero->y > SCREEN_HEIGHT - hero->rect.h) hero->y = SCREEN_HEIGHT - hero->rect.h;

    // Mise à jour du rectangle de collision
    hero->rect.x = (int)hero->x;
    hero->rect.y = (int)hero->y;
    hero->rect.w = HERO_WIDTH;
    hero->rect.h = HERO_HEIGHT;

    // Animation
    if (now - hero->frameTimer >= hero->frameDelay) {
        hero->frame++;
        if (hero->frame >= 6) hero->frame = 0;
        hero->frameTimer = now;
    }

    hero->lastUpdateTime = now;
}

void RenderHero(SDL_Surface *screen, Hero *hero) {
    if (!hero) {
        printf("ERREUR: Pointeur Hero NULL dans RenderHero\n");
        return;
    }

    // Affichage du héros
    SDL_Rect dst = {hero->rect.x, hero->rect.y, 0, 0};
    SDL_BlitSurface(hero->img_per[hero->frame], NULL, screen, &dst);

    // Affichage de la barre de vie
    SDL_BlitSurface(hero->img_vie, &hero->pos_vie_affiche, screen, &hero->pos_vie_ecran);

    // Affichage du score
    sprintf(hero->sc, "Score : %d", hero->score);
    hero->tscore.txt = TTF_RenderText_Blended(hero->tscore.police, hero->sc, hero->tscore.color_txt);
    hero->tscore.pos_txt.x = 1200;
    SDL_BlitSurface(hero->tscore.txt, NULL, screen, &hero->tscore.pos_txt);
}

void FreeHero(Hero *hero) {
    if (!hero) {
        printf("ERREUR: Pointeur Hero NULL dans FreeHero\n");
        return;
    }

    // Libération des sprites
    for (int i = 0; i < 6; i++) {
        if (hero->img_per[i]) {
            SDL_FreeSurface(hero->img_per[i]);
            hero->img_per[i] = NULL;
        }
    }

    // Libération de la barre de vie
    if (hero->img_vie) {
        SDL_FreeSurface(hero->img_vie);
        hero->img_vie = NULL;
    }

    // Libération du texte
    if (hero->tscore.txt) {
        SDL_FreeSurface(hero->tscore.txt);
        hero->tscore.txt = NULL;
    }
    if (hero->tscore.police) {
        TTF_CloseFont(hero->tscore.police);
        hero->tscore.police = NULL;
    }
}

void saut(Hero *hero) {
    if (!hero->isJumping) {
        hero->state = HERO_JUMP;
        hero->isJumping = 1;
        hero->jumpVelocity = -15.0f;
        hero->frame = 0;
    }
}

void augmenter_vitesse(Hero *hero) {
    hero->moveSpeed += hero->acceleration;
    if (hero->moveSpeed >= 40)
        hero->moveSpeed = 40;
}

void diminuer_vitesse(Hero *hero) {
    hero->moveSpeed = 8.0f;
}

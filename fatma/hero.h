#ifndef HERO_H
#define HERO_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "enemy.h"

// Hadi énumération t3 l'états t3 l'héros
enum HeroState {
    HERO_ATTACK, // État t3 l'attaque
    HERO_DEATH,  // État t3 l'mort
    HERO_FALL,   // État t3 l'tombée
    HERO_HIT,    // État t3 l'coup reçu
    HERO_IDLE,   // État t3 l'repos
    HERO_JUMP,   // État t3 l'saut
    HERO_RUN,    // État t3 l'course
    HERO_WALK    // État t3 l'marche
};



// Hadi structure t3 l'héros
typedef struct {
    float x, y;              // Position t3 l'héros (x horizontal, y vertical)
    int direction;           // Direction t3 l'héros (1 droite, -1 gauche)
    enum HeroState state;    // État courant t3 l'héros (mn l'enum)
    int health;              // Santé actuelle t3 l'héros
    int maxHealth;           // Santé maximale t3 l'héros
    float moveSpeed;         // Vitesse t3 l'mouvement
    Animation animations[8]; // Tableau t3 8 animations (wa7da l'kol état)
    SDL_Surface *flippedSpriteSheet; // Image t3 l'héros m3aksa (flipped)
    SDL_Rect rect;           // Rectangle t3 l'héros (position w taille)
    int currentFrame;        // Frame courante fi l'animation
    Uint32 frameTimer;       // Temps t3 l'frame (bch nbdlou l'frames)
    Uint32 frameDelay;       // Délai bin l'frames
    int isJumping;           // 1 ken l'héros ysauter, 0 sinon
    float jumpVelocity;      // Vitesse t3 l'saut
    float gravity;           // Gravité bch l'héros yrj3 lt7t
    int isAttacking;         // 1 ken l'héros y3ml attaque, 0 sinon
    Uint32 lastUpdateTime;   // Dernier temps t3 mise à jour
    int flip;                // 1 ken l'héros m3aks, 0 sinon
    int animationPlaying;    // 1 ken l'animation t3ml, 0 sinon
    int attackDamage;        // Quantité t3 l'dégât t3 l'attaque
    Uint32 lastHitTime;      // Temps du dernier coup reçu
} Hero;

// Fonction bch nbdaw l'héros
void InitHero(Hero *hero, int startX);
// hero: pointeur 3la l'héros
// startX: position initiale x

// Fonction bch nmajiw l'héros
void UpdateHero(Hero *hero, const Uint8 *keys);
// hero: pointeur 3la l'héros
// keys: tableau t3 l'touches pressées

// Fonction bch nrsmou l'héros
void RenderHero(SDL_Surface *screen, Hero *hero);
// screen: l'écran
// hero: pointeur 3la l'héros

// Fonction bch n7rru l'héros
void FreeHero(Hero *hero);
// hero: pointeur 3la l'héros

#endif

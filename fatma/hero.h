#ifndef HERO_H
#define HERO_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "enemy.h"

// États du héros
enum HeroState {
    HERO_ATTACK, // État d'attaque
    HERO_DEATH,  // État de mort
    HERO_FALL,   // État de tombée
    HERO_HIT,    // État de coup reçu
    HERO_IDLE,   // État de repos
    HERO_JUMP,   // État de saut
    HERO_RUN,    // État de course
    HERO_WALK    // État de marche
};

// Structure pour le texte
typedef struct {
    SDL_Surface *txt;
    SDL_Rect pos_txt;
    SDL_Color color_txt;
    TTF_Font *police;
} texte;

// Structure du héros
typedef struct {
    float x, y;              // Position du héros
    int direction;           // Direction (1 droite, -1 gauche)
    enum HeroState state;    // État courant
    int health;              // Santé actuelle
    int maxHealth;           // Santé maximale
    float moveSpeed;         // Vitesse de mouvement
    float acceleration;      // Accélération
    SDL_Surface *img_per[6]; // Sprites du héros
    SDL_Surface *img_vie;    // Image de la barre de vie
    SDL_Rect pos_vie_affiche;// Rectangle pour l'affichage de la vie
    SDL_Rect pos_vie_ecran;  // Position de la barre de vie
    texte tscore;            // Texte du score
    char sc[20];             // Chaîne pour le score
    int score;               // Score actuel
    int frame;               // Frame courante
    int isJumping;           // État du saut
    float jumpVelocity;      // Vitesse du saut
    float gravity;           // Gravité
    int isAttacking;         // État de l'attaque
    Uint32 lastUpdateTime;   // Dernier temps de mise à jour
    Uint32 frameTimer;       // Timer pour les frames
    Uint32 frameDelay;       // Délai entre les frames
    int attackDamage;        // Dégâts d'attaque
    Uint32 lastHitTime;      // Temps du dernier coup reçu
    SDL_Rect rect;           // Rectangle de collision
} Hero;

// Fonctions du héros
void InitHero(Hero *hero, int startX);
void UpdateHero(Hero *hero, const Uint8 *keys);
void RenderHero(SDL_Surface *screen, Hero *hero);
void FreeHero(Hero *hero);
void saut(Hero *hero);
void augmenter_vitesse(Hero *hero);
void diminuer_vitesse(Hero *hero);

#endif

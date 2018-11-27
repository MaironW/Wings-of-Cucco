#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdbool.h>
#include <stdint.h>

// =====================================================

#define PAUSE       5 

// Character directions
#define UP          0
#define RIGHT       1
#define DOWN        2
#define LEFT        3

// Link status for sprites
#define ATTACKING   0
#define WALKING     1
#define HURT        2

// Enemy attacked animation
#define ATTACKED1   4
#define ATTACKED2   5

// Enemy movement style (status)
#define DUMB        0
#define ACTIVE      1
#define FOLLOWER    2

// Warmap status
#define FREE        0   // there's nothing in this place
#define ENEMY       1   // there's an enemy in this spot
#define LINK        8   // Link is there
#define SWORD       16   // Link's sword is there

#define LOSE        9   // some moster attacked Link
#define WIN         17  // Link attacked some monster

// Enemy code
#define GRASS           0
#define CUCCO           1
#define GRAND_CUCCO     2
#define OLDMAN          3
#define GRAND_MADCUCCO  4
#define MADCUCCO        5



// =====================================================
// Main character structure
typedef struct{
    uint8_t x;                         // x coordinate
    uint8_t y;                         // y coordinate
    const unsigned char **sprite[3];   // pointer to the hero sprite sheet
    const unsigned char *last_sprite;  // pointer to the last hero sprite
    const unsigned char *sword;        // pointer to the last sword sprite
    uint8_t size_x;                    // horizontal size of the actual sprite
    uint8_t size_y;                    // vertical size of the actual sprite
    int life;                          // life counter. Goes from 6(full) to 0(dead)
    bool status;                       // 1 is walking. 0 is attacking. 2 is being hurt
    bool step;                         // alternates to make walking effect
    uint8_t direction;                 // to where Link is looking [UP, RIGHT, DOWN, LEFT]
    uint8_t enemies_to_kill;           // how much enemies Link must kill in the level
} Link_t;

// =====================================================
// Standard enemy structure
typedef struct{
    uint8_t x;                         // x coordinate
    uint8_t y;                         // y coordinate
    const unsigned char **sprite[2];   // pointer to the enemy sprite sheet
    const unsigned char *last_sprite;  // pointer to the last enemy sprite
    uint8_t size_x;                    // horizontal size of the actual sprite
    uint8_t size_y;                    // vertical size of the actual sprite
    uint8_t life;                          // life counter (it depends from the enemy)
    uint8_t damage;                    // how much damage the enemy causes to Link
    uint8_t status;                       // 0 is steady. 1 is random walk. 2 follows link
    bool step;                         // alternates to make walking effect
    uint8_t direction;                 // to where the Enemy is looking [UP, RIGHT, DOWN, LEFT]
} Enemy_t;

// =====================================================
// Level structure
typedef struct{
    Link_t link;                        // the hero
    Enemy_t *enemy_queue;               // a queue with all the level monsters
    uint8_t enemy_amount;               // the number of enemies alive in the level
} Level_t;


// =====================================================

uint8_t warmap[48][84]; // pointer to the warmap matrix

#endif


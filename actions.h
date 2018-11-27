#ifndef ACTIONS_H
#define ACTIONS_H

#include "definitions.h"

// =====================================================
// ### GAME INTERACTIONS ###

// Set initial game configurations
void Setup();

// Get the pressed switch information
uint8_t GetSwitch(uint8_t sw);

// Generate the title screen animation
void TitleScreen();

// Generate the selection screen
void SelectionScreen();

// Generate the instruction screen
void InstructionScreen();

// Generate the story screen
void StoryScreen();

// Display the top 3 high scores
void HighScoreScreen();

// Start a new game
void NewGame();

// Update lifebar heart sprites when Link's life change
void Lifebar_Update(uint8_t life);

// Pauses the game. Return 1 if quit. 0 if not
bool Pause();

// Link must kill as many monsters as he can
void SurvivorMode();

// Put the number of killed enemies on screen
void DisplayScore();

// =====================================================
// ### LEVEL INTERACTIONS ###

// Set a new level.
// returns 1 when finished, 0 when GameOver
bool Level_New(Enemy_t *queue, uint8_t n_monsters);

// Creates the warmap for the level
void Level_WarMapStart(uint8_t warmap[48][84]);

// Updates the warmap when Link or an enemy moves
void Level_WarMapUpdate(Link_t *link, Enemy_t *enemy, const unsigned char *sprite, uint8_t x, uint8_t y, uint8_t value);

// Clears the warmap
void Level_WarMapClear(const unsigned char *sprite, uint8_t x, uint8_t y, uint8_t value);

// Returns which monster is being attacked or attacking
uint8_t Level_WarMapCheckEnemy(Enemy_t *enemy, uint8_t status);

// =====================================================
// ### LINK ACTIONS ####

// Creates a hero at the center of the display
Link_t Link_New();

// Change the hero position and sprite
void Link_Move(Link_t *link, Enemy_t *enemy);

// Set the hero to attack mode
void Link_Attack(Link_t *link, Enemy_t *enemy);

// Link loses the same amount of life that the enemy's damage value
void Link_LifeLoss(Link_t *link, uint8_t damage);

// Link change his position when attacked
void Link_IsAttacked(Link_t *link, Enemy_t *enemy);

// Game Over
void Link_IsDead();


// =====================================================
// ### ENEMY ACTIONS ###

// Summon up a new enemy on a fixed place at the display
// IT WORKS JUST FOR GRASS BY NOW
Enemy_t Enemy_New(uint8_t index, uint8_t x, uint8_t y, uint8_t life, uint8_t damage, uint8_t status);

// Change the enemy position and sprite
// Some enemies will follow Link, some have pattern moves
void Enemy_Move(Link_t *link, Enemy_t *enemy);

#endif

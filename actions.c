#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"

#include "actions.h"
#include "buttons.h"
#include "bitmaps.h"
#include "Nokia5110.h"

// =====================================================
// ### GLOBAL VARIABLES ###

// Global life, to keep the value through all the levels
uint8_t global_life = 6;

// Number of enemies killed in survivor mode
uint8_t survivor_points = 0;

// High score with number of monsters killed in survivor mode
uint8_t highscore[3]={0,0,0};

// Indicates Story Mode or Survival Mode
bool mode = 0;

// =====================================================
// ### GAME INTERACTIONS ###

// Set initial game configurations
void Setup(){

    // Configures the clock to run in 12.5 MHz
    // A Game Boy game runs in 4 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_20 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    // Configures SysTick for random numbers
    SysTickPeriodSet(1000);
    SysTickEnable();

    // Display setup
    Nokia5110_Init();
    Nokia5110_Clear();
    Nokia5110_ClearBuffer();

    // Buttons setup
    ConfigureButtons();

    // Initialize the warmap
    Level_WarMapStart(warmap);
}

// Get the pressed switch information
uint8_t GetSwitch(uint8_t sw){
    switch(sw){
        case 12: return UP;
        case 23: return RIGHT;
        case 32: return DOWN;
        case 21: return LEFT;
        case 44: return SWORD;
        case 14: return PAUSE;
        default: break;
    }
    return BUTTON_NOT_PRESSED;
}

// Generate the title screen animation
void TitleScreen(){
    InstructionScreen();
    while(1){
        Nokia5110_Clear();
        Nokia5110_ClearBuffer();

        Nokia5110_PrintBMP(25,5,signature,0);
        Nokia5110_PrintBMP(10,35,TitleLogo,0);

        Nokia5110_DisplayBuffer();

        Nokia5110_PrintBMP(64,22,defeated_1,0);
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet() / 15);
        Nokia5110_PrintBMP(64,22,defeated_2,0);
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet() / 15);
        Nokia5110_PrintBMP(64,22,defeated_3,0);
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet() / 15);

        while(GetSwitch(GetButton())==BUTTON_NOT_PRESSED){
            Nokia5110_PrintBMP(64,22,cucco_left_1,0);
            Nokia5110_DisplayBuffer();
            SysCtlDelay(SysCtlClockGet() / 15);
            if(GetSwitch(GetButton())!=BUTTON_NOT_PRESSED) break;
            Nokia5110_PrintBMP(64,22,cucco_left_2,0);
            Nokia5110_DisplayBuffer();
            SysCtlDelay(SysCtlClockGet() / 15);
        }
        Nokia5110_PrintBMP(64,22,defeated_3,0);
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet() / 15);
        Nokia5110_PrintBMP(64,22,defeated_2,0);
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet() / 15);
        Nokia5110_ClearBitmap(64,22,cucco_left_1);
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet() / 15);

        Nokia5110_Clear();
        SysCtlDelay(SysCtlClockGet() / 15);
        Nokia5110_ClearBuffer();

        SelectionScreen();
    }
}

// Generate the selection screen
void SelectionScreen(){
    uint8_t i=0;
    Nokia5110_PrintBMP(0,47,menubg,0);
    Nokia5110_PrintBMP(25,43,menu_options,0);
    Nokia5110_PrintBMP(20,7,seta,0);
    Nokia5110_DisplayBuffer();
    while(GetSwitch(GetButton())!=BUTTON_NOT_PRESSED){}

    while(1){
        switch(GetSwitch(GetButton())){
            case DOWN:
                if(i<4){
                    Nokia5110_PrintBMP(20,7+9*i,blackseta,0);
                    i++;
                    Nokia5110_PrintBMP(20,7+9*i,seta,0);
                    Nokia5110_DisplayBuffer();
                    SysCtlDelay(SysCtlClockGet() / 10);
                }
                break;

            case UP:
                if(i>0){
                    Nokia5110_PrintBMP(20,7+9*i,blackseta,0);
                    i--;
                    Nokia5110_PrintBMP(20,7+9*i,seta,0);
                    Nokia5110_DisplayBuffer();
                    SysCtlDelay(SysCtlClockGet() / 10);
                }
                break;

            case SWORD:
                Nokia5110_ClearBuffer();
                if(i==0) StoryScreen();
                if(i==1){
                    NewGame();
                    SysCtlDelay(SysCtlClockGet()/7);
                    return;
                }
                if(i==2){
                     SurvivorMode();
                     SysCtlDelay(SysCtlClockGet()/7);
                     return;
                }
                if(i==3) HighScoreScreen();
                if(i==4) InstructionScreen();
                i=0;
                SysCtlDelay(SysCtlClockGet()/7);
                Nokia5110_PrintBMP(0,47,menubg,0);
                Nokia5110_PrintBMP(25,43,menu_options,0);
                Nokia5110_PrintBMP(20,7,seta,0);
                Nokia5110_DisplayBuffer();
                break;

            case PAUSE:
                return;
        }
    }
}

// Generate the instruction screen
void InstructionScreen(){
    SysCtlDelay(SysCtlClockGet()/5);
    Nokia5110_PrintBMP(0,47,menubg,0);
    Nokia5110_PrintBMP(20,6,instructions,0);
    Nokia5110_PrintBMP(20,27,buttonmtx,0);
    while(1){
        Nokia5110_PrintBMP(27,12,button0,0);    // UP
        Nokia5110_PrintBMP(34,17,button0,0);    // RIGHT
        Nokia5110_PrintBMP(27,22,button0,0);    // DOWN
        Nokia5110_PrintBMP(20,17,button0,0);    // LEFT
        Nokia5110_PrintBMP(41,27,button0,0);    // SWORD
        Nokia5110_PrintBMP(41,12,button0,0);    // PAUSE

        Nokia5110_PrintBMP(50,43,link_left_2,0);   // LINK

        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet()/7);

        if(GetSwitch(GetButton())==SWORD) break;
        Nokia5110_PrintBMP(28,43,blackseta,0);
        Nokia5110_PrintBMP(32,43,blackseta,0);
        Nokia5110_PrintBMP(36,43,blackseta,0);
        Nokia5110_PrintBMP(40,43,blackseta,0);
        Nokia5110_PrintBMP(27,12,button1,0);    // UP
        Nokia5110_PrintBMP(20,43,up,0);
        Nokia5110_PrintBMP(50,43,link_up_1,0);   // LINK
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet()/3);

        if(GetSwitch(GetButton())==SWORD) break;
        Nokia5110_PrintBMP(27,12,button0,0);    // UP
        Nokia5110_PrintBMP(34,17,button1,0);    // RIGHT
        Nokia5110_PrintBMP(20,43,right,0);
        Nokia5110_PrintBMP(50,43,link_right_1,0);   // LINK
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet()/3);

        if(GetSwitch(GetButton())==SWORD) break;
        Nokia5110_PrintBMP(34,17,button0,0);    // RIGHT
        Nokia5110_PrintBMP(27,22,button1,0);    // DOWN
        Nokia5110_PrintBMP(20,43,down,0);
        Nokia5110_PrintBMP(50,43,link_down_1,0);   // LINK
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet()/3);

        if(GetSwitch(GetButton())==SWORD) break;
        Nokia5110_PrintBMP(36,43,blackseta,0);
        Nokia5110_PrintBMP(27,22,button0,0);    // DOWN
        Nokia5110_PrintBMP(20,17,button1,0);    // LEFT
        Nokia5110_PrintBMP(20,43,left,0);
        Nokia5110_PrintBMP(50,43,link_left_1,0);   // LINK
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet()/3);

        if(GetSwitch(GetButton())==SWORD) break;
        Nokia5110_PrintBMP(20,17,button0,0);    // LEFT
        Nokia5110_PrintBMP(41,27,button1,0);    // SWORD
        Nokia5110_PrintBMP(20,43,attack,0);
        Nokia5110_PrintBMP(50,43,link_left_attack,0);   // LINK
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet()/3);

        if(GetSwitch(GetButton())==SWORD) break;
        Nokia5110_PrintBMP(40,43,blackseta,0);
        Nokia5110_PrintBMP(41,27,button0,0);    // SWORD
        Nokia5110_PrintBMP(41,12,button1,0);    // PAUSE
        Nokia5110_PrintBMP(20,43,pause,0);
        Nokia5110_PrintBMP(50,43,link_left_attack,0);   // LINK
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet()/3);

    }
    Nokia5110_Clear();
    Nokia5110_ClearBuffer();
}

// Generate the story screen
void StoryScreen(){
    uint8_t i=0;    // counter for animation

    SysCtlDelay(SysCtlClockGet()/5);
    Nokia5110_PrintBMP(0,47,menubg,0);
    // Link's friend Malon asks for help when her pet Cucco disappeared.
    Nokia5110_PrintBMP(18,42,story_1,0);
    Nokia5110_DisplayBuffer();
    SysCtlDelay(SysCtlClockGet()/5);

    while(GetSwitch(GetButton())==BUTTON_NOT_PRESSED){}
    // shows up Malons house background
    Nokia5110_PrintBMP(0,47,malon_house,0);
    Nokia5110_DisplayBuffer();

    // shows up Malon asking for help
    for(i=0;i<16;i++){
        Nokia5110_PrintBMP(40,30,malon_sprite[i%4],0);
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet()/15);
    }

    Nokia5110_PrintBMP(0,47,menubg,0);
    // Being a hero, it's his duty to rescue the bird.
    Nokia5110_PrintBMP(18,34,story_2,0);
    Nokia5110_DisplayBuffer();
    SysCtlDelay(SysCtlClockGet()/5);

    while(GetSwitch(GetButton())==BUTTON_NOT_PRESSED){}

    Nokia5110_PrintBMP(0,47,malon_house,0);
    Nokia5110_DisplayBuffer();

    for(i=0;i<16;i++){
        Nokia5110_PrintBMP(40,30,malon_sprite[i%4],0);
        Nokia5110_PrintBMP(35,46,link_walk_1[i%4],0);
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet()/15);
    }


    Nokia5110_PrintBMP(0,47,menubg,0);
    // But... there's more than he thought happening outside their village.
    Nokia5110_PrintBMP(18,42,story_3,0);
    Nokia5110_DisplayBuffer();
    SysCtlDelay(SysCtlClockGet()/5);
    while(GetSwitch(GetButton())==BUTTON_NOT_PRESSED){}

    // Nokia5110_PrintBMP(0,47,menubg,0);
    Nokia5110_Clear();
    Nokia5110_ClearBuffer();
    Nokia5110_DisplayBuffer();
    for(i=0;i<16;i++){
        Nokia5110_PrintBMP(35,26,oldman_array[i%2][DOWN],0);
        Nokia5110_PrintBMP(36,42,poison,0);
        Nokia5110_PrintBMP(10,20,candle_sprites[i%4],0);
        Nokia5110_PrintBMP(60,20,candle_sprites[i%4],0);
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet()/15);
    }
}

// Display the top 3 high scores
void HighScoreScreen(){
    SysCtlDelay(SysCtlClockGet()/5);
    Nokia5110_PrintBMP(0,47,menubg,0);
    Nokia5110_PrintBMP(25,37,highscoremenu,0);


    int i,d1,d2;
    for(i=0;i<3;i++){
        d2 = highscore[i]%10;
        d1 = highscore[i]/10;
        Nokia5110_PrintBMP(36,20+9*i,number[d1],0);
        Nokia5110_PrintBMP(40,20+9*i,number[d2],0);
    }




    Nokia5110_DisplayBuffer();
    SysCtlDelay(SysCtlClockGet()/5);

    while(GetSwitch(GetButton())==BUTTON_NOT_PRESSED){}
}

// Start a new game
//  good positions for enemies
//  0,16  16,16  32,16  48,16  64,16
//  LINK  16,31  32,31  48,31  64,31
//  0,47  16,47  32,47  48,47  64,47
void NewGame(){

    Nokia5110_Clear();
    SysCtlDelay(SysCtlClockGet()/7);
    mode = 0;
    global_life = 6;

    // number of monsters in each level
    // must be passed as an argument to Level_New
    uint8_t n;

    // monster queue
    Enemy_t queue[6]={0,0,0,0,0,0};

    uint8_t i;  // frame counter for cutscenes

    // ========================================
    // LEVEL 1                  [Grass Cutting]
     n = 4;
     queue[0] = Enemy_New(GRASS,32,15,1,0,DUMB);
     queue[1] = Enemy_New(GRASS,32,31,1,0,DUMB);
     queue[2] = Enemy_New(GRASS,32,47,1,0,DUMB);
     queue[3] = Enemy_New(GRASS,48,31,1,0,DUMB);
     if(!(Level_New(queue,n))) return;

     // ========================================
     // CUTSCENE 1              [Cucco Run Away]
     queue[0] = Enemy_New(CUCCO,48,31,1,3,ACTIVE);

     Nokia5110_PrintBMP(16,47,grass_alive,0);
     Nokia5110_PrintBMP(32,15,grass_alive,0);
     Nokia5110_PrintBMP(48,31,cucco_right_1,0);
     Nokia5110_PrintBMP(1,33,link_right_1,0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/20);
     Nokia5110_PrintBMP(48,31,cucco_right_2,0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/20);
     Nokia5110_PrintBMP(48,31,cucco_left_1,0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/20);
     Nokia5110_PrintBMP(48,31,cucco_left_2,0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/20);

     while(queue[0].x<MAX_X-15){
         Nokia5110_ClearBitmap(queue[0].x,queue[0].y,queue[0].last_sprite);
         Nokia5110_DisplayBuffer();
         queue[0].x +=2;
         queue[0].direction = RIGHT;
         queue[0].last_sprite = queue[0].sprite[queue[0].step][queue[0].direction];
         queue[0].step = !queue[0].step;
         Nokia5110_PrintBMP(queue[0].x,queue[0].y,queue[0].last_sprite,0);
         Nokia5110_DisplayBuffer();
         SysCtlDelay(SysCtlClockGet()/25);
     }

     // ========================================
     // LEVEL 2                    [Cucco Found]
     n = 2;
     queue[0] = Enemy_New(GRASS,16,47,1,0,DUMB);
     queue[1] = Enemy_New(GRASS,32,15,1,0,DUMB);
     if(!(Level_New(queue,n))) return;

     // ========================================
     // CUTSCENE 2                 [The Old Man]

     Nokia5110_PrintBMP(1,33,link_right_1,0);
     queue[0] = Enemy_New(OLDMAN,64,31,3,1,DUMB);
     queue[1] = Enemy_New(CUCCO,48,31,3,1,DUMB);

     for(i=0;i<10;i++){

         queue[0].last_sprite = queue[0].sprite[queue[0].step][LEFT];
         queue[0].step = !queue[0].step;
         queue[1].last_sprite = queue[1].sprite[queue[1].step][RIGHT];
         queue[1].step = !queue[1].step;

         Nokia5110_PrintBMP(64,31,queue[0].last_sprite,0);
         Nokia5110_PrintBMP(48,31,queue[1].last_sprite,0);
         Nokia5110_DisplayBuffer();
         SysCtlDelay(SysCtlClockGet()/20);
     }

     Nokia5110_PrintBMP(48,31,defeated_3,0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/10);
     Nokia5110_PrintBMP(48,31,defeated_2,0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/10);
     Nokia5110_PrintBMP(48,31,defeated_1,0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/10);

     Nokia5110_PrintBMP(48,31,defeated_1,0);
     Nokia5110_PrintBMP(32,16,defeated_1,0);
     Nokia5110_PrintBMP(64,47,defeated_1,0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/10);
     Nokia5110_PrintBMP(48,31,defeated_2,0);
     Nokia5110_PrintBMP(32,16,defeated_2,0);
     Nokia5110_PrintBMP(64,47,defeated_2,0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/10);
     Nokia5110_PrintBMP(48,31,defeated_3,0);
     Nokia5110_PrintBMP(32,16,defeated_3,0);
     Nokia5110_PrintBMP(64,47,defeated_3,0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/10);
     Nokia5110_PrintBMP(48,31,queue[1].sprite[queue[1].step][LEFT],0);
     Nokia5110_PrintBMP(32,16,queue[1].sprite[queue[1].step][RIGHT],0);
     Nokia5110_PrintBMP(64,47,queue[1].sprite[queue[1].step][LEFT],0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/10);

     Nokia5110_ClearBitmap(64,31,queue[0].sprite[queue[0].step][LEFT]);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/10);

     // ========================================
     // LEVEL 3                [Tripple Trouble]
     n = 3;
     queue[0] = Enemy_New(CUCCO,48,31,3,1,FOLLOWER);
     queue[1] = Enemy_New(CUCCO,32,16,3,1,FOLLOWER);
     queue[2] = Enemy_New(CUCCO,64,47,3,1,ACTIVE);
     if(!(Level_New(queue,n))) return;

     // ========================================
     // LEVEL 4                 [Quadcoptrouble]
     n = 4;
     queue[0] = Enemy_New(CUCCO,48,16,3,1,ACTIVE);
     queue[1] = Enemy_New(CUCCO,32,47,3,1,FOLLOWER);
     queue[2] = Enemy_New(CUCCO,64,47,3,1,ACTIVE);
     queue[3] = Enemy_New(CUCCO,64,31,3,1,FOLLOWER);
     if(!(Level_New(queue,n))) return;

     // ========================================
     // LEVEL 5                   [Cucco's Five]
     n = 5;
     queue[0] = Enemy_New(CUCCO,16,47,3,1,ACTIVE);
     queue[1] = Enemy_New(CUCCO,32,16,3,1,FOLLOWER);
     queue[2] = Enemy_New(CUCCO,32,47,3,1,ACTIVE);
     queue[3] = Enemy_New(CUCCO,64,47,3,1,ACTIVE);
     queue[4] = Enemy_New(CUCCO,48,16,3,1,ACTIVE);
     if(!(Level_New(queue,n))) return;

     // ========================================
     // CUTSCENE 3            [Grand Cucco Born]

     Nokia5110_PrintBMP(1,33,link_right_1,0);
     queue[0] = Enemy_New(CUCCO,64,16,3,1,ACTIVE);

     for(i=0;i<10;i++){
         queue[0].last_sprite = queue[0].sprite[queue[0].step][LEFT];
         queue[0].step = !queue[0].step;

         Nokia5110_PrintBMP(64,16,cucco_array[queue[0].step][LEFT],0);
         Nokia5110_PrintBMP(32,16,cucco_array[queue[0].step][LEFT],0);
         Nokia5110_PrintBMP(32,47,cucco_array[queue[0].step][LEFT],0);
         Nokia5110_PrintBMP(48,31,cucco_array[queue[0].step][LEFT],0);
         Nokia5110_PrintBMP(64,47,cucco_array[queue[0].step][LEFT],0);
         Nokia5110_PrintBMP(16,31,oldman_array[queue[0].step][RIGHT],0);
         Nokia5110_DisplayBuffer();
         SysCtlDelay(SysCtlClockGet()/20);
     }

     Nokia5110_PrintBMP(48,31,defeated_3,0);
     Nokia5110_PrintBMP(64,16,defeated_3,0);
     Nokia5110_PrintBMP(64,47,defeated_3,0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/10);
     Nokia5110_PrintBMP(48,31,defeated_2,0);
     Nokia5110_PrintBMP(64,16,defeated_2,0);
     Nokia5110_PrintBMP(64,47,defeated_2,0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/10);
     Nokia5110_PrintBMP(48,31,defeated_1,0);
     Nokia5110_PrintBMP(64,16,defeated_1,0);
     Nokia5110_PrintBMP(64,47,defeated_1,0);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/10);
     Nokia5110_ClearBitmap(48,31,defeated_1);
     Nokia5110_ClearBitmap(64,16,defeated_1);
     Nokia5110_ClearBitmap(64,47,defeated_1);
     Nokia5110_ClearBitmap(16,31,defeated_1);
     Nokia5110_DisplayBuffer();
     SysCtlDelay(SysCtlClockGet()/10);

     queue[2] = Enemy_New(GRAND_CUCCO,48,40,3,1,ACTIVE);

     // ========================================
     // LEVEL 6                    [Grand Cucco]
     n = 3;
     queue[0] = Enemy_New(CUCCO,32,16,3,1,FOLLOWER);
     queue[1] = Enemy_New(CUCCO,32,47,3,1,FOLLOWER);
     queue[2] = Enemy_New(GRAND_CUCCO,48,40,6,3,ACTIVE);
     if(!(Level_New(queue,n))) return;

     // ========================================
     // LEVEL 7                        [Old Man]
     n = 4;
     queue[0] = Enemy_New(OLDMAN,48,31,9,4,FOLLOWER);
     queue[1] = Enemy_New(GRASS,32,15,1,0,DUMB);
     queue[2] = Enemy_New(GRASS,16,47,1,0,DUMB);
     queue[3] = Enemy_New(GRASS,64,15,1,0,DUMB);
     if(!(Level_New(queue,n))) return;

    // ========================================
    // LEVEL 8                 [GrandMad Cucco]
    n = 1;
    queue[0] = Enemy_New(GRAND_MADCUCCO,48,47,12,5,FOLLOWER);
    if(!(Level_New(queue,n))) return;

    // ========================================
    // CUTSCENE 5          [Everything is Fine]

    queue[0] = Enemy_New(MADCUCCO,50,46,20,0,FOLLOWER);
    Nokia5110_PrintBMP(1,33,link_right_1,0);
    Nokia5110_ClearBitmap(0,7,lifebar_heart[0]);
    Nokia5110_ClearBitmap(8,7,lifebar_heart[1]);
    Nokia5110_ClearBitmap(16,7,lifebar_heart[2]);
    Nokia5110_DisplayBuffer();
    SysCtlDelay(SysCtlClockGet()/5);

    Nokia5110_PrintBMP(16,14,thankyoulink,0);
    for(i=0;i<12;i++){
        Nokia5110_PrintBMP(60,20,malon_sprite[i%4],0);
        Nokia5110_PrintBMP(50,46,queue[0].last_sprite,0);
        queue[0].step = !queue[0].step;
        queue[0].last_sprite = queue[0].sprite[queue[0].step][i%4];
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet()/10);
    }

    Nokia5110_ClearBitmap(16,14,thankyoulink);
    Nokia5110_PrintBMP(16,40,butialready,0);
    for(i=0;i<12;i++){
        Nokia5110_PrintBMP(60,20,malon_sprite[i%4],0);
        Nokia5110_PrintBMP(50,46,queue[0].last_sprite,0);
        queue[0].step = !queue[0].step;
        queue[0].last_sprite = queue[0].sprite[queue[0].step][i%4];
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet()/10);
    }

    Nokia5110_ClearBitmap(16,40,butialready);
    Nokia5110_PrintBMP(16,46,playwithhim,0);
    for(i=0;i<12;i++){
        Nokia5110_PrintBMP(60,20,malon_sprite[i%4],0);
        Nokia5110_PrintBMP(50,46,queue[0].last_sprite,0);
        queue[0].step = !queue[0].step;
        queue[0].last_sprite = queue[0].sprite[queue[0].step][i%4];
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet()/10);
    }
    Nokia5110_ClearBitmap(60,20,malon_sprite[0]);
    Nokia5110_ClearBitmap(16,46,playwithhim);
    Nokia5110_DisplayBuffer();

    // ========================================
    // LEVEL 9                      [THE END]
    n = 1;
    if(!(Level_New(queue,n))) return;
    // ========================================
    // CUTSCENE 6                     [The End]
    Nokia5110_Clear();
    Nokia5110_ClearBuffer();
    SysCtlDelay(SysCtlClockGet()/4);
    Nokia5110_PrintBMP(24,30,thanks,0);
    Nokia5110_DisplayBuffer();
    while(GetSwitch(GetButton())==BUTTON_NOT_PRESSED){}
}

// Update lifebar heart sprites when Link's life change
void Lifebar_Update(uint8_t life){
    switch(life){
        case 6:
            lifebar_heart[0] = heart_full;
            lifebar_heart[1] = heart_full;
            lifebar_heart[2] = heart_full;
            break;
        case 5:
            lifebar_heart[0] = heart_full;
            lifebar_heart[1] = heart_full;
            lifebar_heart[2] = heart_half;
            break;
        case 4:
            lifebar_heart[0] = heart_full;
            lifebar_heart[1] = heart_full;
            lifebar_heart[2] = heart_empty;
            break;
        case 3:
            lifebar_heart[0] = heart_full;
            lifebar_heart[1] = heart_half;
            lifebar_heart[2] = heart_empty;
            break;
        case 2:
            lifebar_heart[0] = heart_full;
            lifebar_heart[1] = heart_empty;
            lifebar_heart[2] = heart_empty;
            break;
        case 1:
            lifebar_heart[0] = heart_half;
            lifebar_heart[1] = heart_empty;
            lifebar_heart[2] = heart_empty;
            break;
        default:
            lifebar_heart[0] = heart_empty;
            lifebar_heart[1] = heart_empty;
            lifebar_heart[2] = heart_empty;
            break;
    }

    Nokia5110_PrintBMP(0,7,lifebar_heart[0],0);
    Nokia5110_PrintBMP(8,7,lifebar_heart[1],0);
    Nokia5110_PrintBMP(16,7,lifebar_heart[2],0);
    Nokia5110_DisplayBuffer();
}

// Pauses the game and asks for continue or quit
bool Pause(){
    uint8_t i=0;
    while(GetSwitch(GetButton())==PAUSE){}

    Nokia5110_PrintBMP(34,8,pausemenu,0);
    Nokia5110_PrintBMP(54,6,invseta,0);
    Nokia5110_DisplayBuffer();
    while(1){
        switch(GetSwitch(GetButton())){
            case LEFT:
                if(i){
                    Nokia5110_ClearBitmap(71,6,invseta);
                    i=!i;
                    Nokia5110_PrintBMP(54,6,invseta,0);
                    Nokia5110_DisplayBuffer();
                    SysCtlDelay(SysCtlClockGet() / 10);
                }
                break;

            case RIGHT:
                if(!i){
                    Nokia5110_ClearBitmap(54,6,invseta);
                    i=!i;
                    Nokia5110_PrintBMP(71,6,invseta,0);
                    Nokia5110_DisplayBuffer();
                    SysCtlDelay(SysCtlClockGet() / 10);
                }
                break;

            case SWORD:
                SysCtlDelay(SysCtlClockGet()/10);
                Nokia5110_ClearBitmap(34,8,pausemenu);
                return i;
        }
    }
}

// Link must kill as many monsters as he can
void SurvivorMode(){
    uint8_t enemy[4]={GRASS,CUCCO,OLDMAN,MADCUCCO};
    uint8_t boss[2]={GRAND_CUCCO,GRAND_MADCUCCO};
    uint8_t i;  // simple counter

    Nokia5110_Clear();
    SysCtlDelay(SysCtlClockGet()/7);

    mode = 1;
    survivor_points = 0;
    global_life = 6;

    // number of monsters in each level
    // must be passed as an argument to Level_New
    uint8_t n;

    uint8_t m;  // kind of monster
    uint8_t s;  // monster status

    // monster queue
    Enemy_t queue[5]={0,0,0,0,0};

    // generates random levels at each iteration
    while(1){
        srand(SysTickValueGet());
        n = rand()%4+1;
        if(n==1){
            m = rand()%2;
            s = rand()%2+1;
            queue[0] = Enemy_New(boss[m],30,40,5*boss[m],1,s);
        }else{
            for(i=0;i<n;i++){
                srand(SysTickValueGet());
                m = rand()%4;
                if(m==0) s=0;
                else s = rand()%2+1;
                queue[i] = Enemy_New(enemy[m],20+16*i,47-2*(s+1)*i-m,3*enemy[m]+1,1,s);
            }
        }
        if(!(Level_New(queue,n))) break;
    }

    if(survivor_points>highscore[0]){
        highscore[2]=highscore[1];
        highscore[1]=highscore[0];
        highscore[0]=survivor_points;
    }else if(survivor_points>highscore[1]){
        highscore[2]=highscore[1];
        highscore[1]=survivor_points;
    }else if(survivor_points>highscore[2]){
        highscore[2]=survivor_points;
    }

    Nokia5110_Clear();
    Nokia5110_ClearBuffer();
    return;
}

// Put the number of killed enemies on screen
void DisplayScore(){
    int d1,d2;

    d2 = survivor_points%10;
    d1 = survivor_points/10;

    Nokia5110_PrintBMP(73,7,blackseta,0);
    Nokia5110_PrintBMP(73,5,blackseta,0);
    Nokia5110_PrintBMP(74,7,number[d1],0);
    Nokia5110_PrintBMP(78,7,number[d2],0);
    Nokia5110_DisplayBuffer();
}

// =====================================================
// ### LEVEL INTERACTIONS ###

// Set a new level
// returns 1 when finished, 0 when GameOver
bool Level_New(Enemy_t *queue, uint8_t n_monsters){

    uint8_t n;                  // index for the enemies
    Level_WarMapStart(warmap);  // Initialize the warmap
    Level_t level;              // creates a new level
    level.link = Link_New();    // creates a new Link for the level
    level.link.enemies_to_kill = n_monsters;    // set how many monsters Link must defeat to finish the level

    // put Link in the warmap
    Level_WarMapUpdate(&(level.link),level.enemy_queue,level.link.last_sprite,level.link.x,level.link.y,LINK);

    Lifebar_Update(global_life);              // set and show up the lifebar on the screen
    level.enemy_amount = n_monsters; // how many monsters in the level
    level.enemy_queue = queue;  // set the enemy queue to the level

    // update the warmap with current enemies position
    for(n=0;n<n_monsters;n++){
        Level_WarMapUpdate(&(level.link),level.enemy_queue,level.enemy_queue[n].last_sprite,level.enemy_queue[n].x,level.enemy_queue[n].y,ENEMY);
    }

    // the level will run inside this loop
    while(level.link.enemies_to_kill){

        // change Link's position and attitude
        // Link is not in the warmap until it moves or attacks
        Link_Move(&(level.link), level.enemy_queue);
        SysCtlDelay(SysCtlClockGet() / 40);

        // change all the enemies position
        Enemy_Move(&(level.link), level.enemy_queue);

        // overlap the screen with lifebar
        Lifebar_Update(level.link.life);

        // show score if in Survival mode
        if(mode) DisplayScore();

        if(level.link.life<=0) return 0;
    }

    // level finished animation
    while(level.link.x<MAX_X){
        Nokia5110_ClearBitmap(level.link.x,level.link.y,level.link.sprite[WALKING+level.link.step][RIGHT]);
        level.link.x +=2;
        level.link.step = !level.link.step;
        Nokia5110_PrintBMP(level.link.x,level.link.y,level.link.sprite[WALKING+level.link.step][RIGHT],0);
        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet() / 35);
    }

    global_life = level.link.life;
    Nokia5110_Clear();
    return 1;
}

// Creates the warmap for the level
void Level_WarMapStart(uint8_t warmap[48][84]){
    uint8_t i,j;
    for(i=0;i<48;i++){
        for(j=0;j<84;j++){
            warmap[i][j] = 0;
        }
    }
}

// Updates the warmap when Link moves
void Level_WarMapUpdate(Link_t *link, Enemy_t *enemy, const unsigned char *sprite, uint8_t x, uint8_t y, uint8_t value){
    uint8_t i,j;    // index to make through the warmap
    uint8_t m;      // actual enemy index
    uint8_t dx = Nokia5110_getWidth(sprite);    // return horizontal sprite size
    uint8_t dy = Nokia5110_getHeight(sprite);   // return vertical sprite size

    uint8_t status = FREE;  // current status will start as free (no objects in the spot)

    // walk through the warmap inside the sprite coordinates
    for(i = y; i > y-dy ; i--){
        for(j = x; j < x+dx; j++){
            warmap[i][j] += value;  // add the character value to the warmap
            // change the status if the result is one of the status options
            if(warmap[i][j]==LOSE){
                status = LOSE;
            }
            if(warmap[i][j]==WIN){
                status = WIN;
            }
        }
    }

    // check the final status
    switch(status){

        case LOSE:
            m = Level_WarMapCheckEnemy(enemy,status); // discover which enemy attacked Link
            Link_IsAttacked(link,&enemy[m]);          // link loses life
            break;

        case WIN:
            m = Level_WarMapCheckEnemy(enemy,status); // discover which enemy Link attacked
            Link_Attack(link,&enemy[m]);
            // Level_WarMapUpdate(link, enemy, link->last_sprite, link->x, link->y, LINK);
            break;

        default: break;
    }
}

// Clears the warmap
void Level_WarMapClear(const unsigned char *sprite, uint8_t x, uint8_t y, uint8_t value){
    uint8_t i,j;
    uint8_t dx = Nokia5110_getWidth(sprite);
    uint8_t dy = Nokia5110_getHeight(sprite);

    for(i = y; i > y-dy ; i--){
        for(j = x; j < x+dx; j++){
            warmap[i][j]-=value;
        }
    }
}

// Returns which monster is being attacked or attacking
uint8_t Level_WarMapCheckEnemy(Enemy_t *enemy, uint8_t status){
    uint8_t i,j;
    uint8_t m=0; // monster index

    // there will be no more than 6 enemies at a level
    for(m=0;m<6;m++){
        for(i=enemy[m].x; i<=enemy[m].x+enemy[m].size_x; i++){
            for(j=enemy[m].y; j>enemy[m].y-enemy[m].size_y; j--){
                if(warmap[j][i]==status){
                    return m;
                }
            }
        }
    }

    return 99;
}


// =====================================================
// ### LINK ACTIONS ####

// Creates a hero at the center of the display
Link_t Link_New(){
    Link_t hero;
    hero.x = 1;
    hero.y = 33;
    hero.sprite[0] = link_attack;
    hero.sprite[1] = link_walk_1;
    hero.sprite[2] = link_walk_2;
    hero.last_sprite = link_walk_1[RIGHT];
    hero.sword = sword_sprite[RIGHT];
    hero.size_x = 14;
    hero.size_y = 16;
    hero.life = global_life;
    hero.status = WALKING;
    hero.step = 0;
    hero.direction = RIGHT;

    Nokia5110_PrintBMP(hero.x, hero.y,hero.last_sprite,0);
    Nokia5110_DisplayBuffer();

    return hero;
}

// Change the hero position and sprite
void Link_Move(Link_t *link, Enemy_t *enemy){
    switch(GetSwitch(GetButton())){
        case UP:
            Nokia5110_ClearBitmap(link->x,link->y,link->last_sprite);
            Level_WarMapClear(link->last_sprite, link->x, link->y, LINK);
            Nokia5110_DisplayBuffer();

            // change Link position if it is not in the screen border
            if(link->y > link->size_y + 2) link->y-=2;
            // if Link go out of the screen, change it's position to the border
            else link->y = link->size_y;

            link->direction = UP; // change Link's direction
            // updates Link last sprite
            link->last_sprite = link->sprite[WALKING+link->step][link->direction];
            Nokia5110_PrintBMP(link->x,link->y,link->last_sprite,0);
            Nokia5110_DisplayBuffer();
            link->step = !(link->step); // alternate Link step for sprite animation
            Level_WarMapUpdate(link, enemy, link->last_sprite, link->x, link->y, LINK);
            break;

        case RIGHT:
            Nokia5110_ClearBitmap(link->x,link->y,link->last_sprite);
            Level_WarMapClear(link->last_sprite, link->x, link->y, LINK);
            Nokia5110_DisplayBuffer();

            if(link->x < MAX_X - link->size_x - 2) link->x+=2;
            else link->x = MAX_X - link->size_x - 1;

            link->direction = RIGHT;
            link->last_sprite = link->sprite[WALKING+link->step][link->direction];
            Nokia5110_PrintBMP(link->x,link->y,link->last_sprite,0);
            Nokia5110_DisplayBuffer();
            link->step = !(link->step);
            Level_WarMapUpdate(link, enemy, link->last_sprite, link->x, link->y, LINK);
            break;

        case DOWN:
            Nokia5110_ClearBitmap(link->x,link->y,link->last_sprite);
            Level_WarMapClear(link->last_sprite, link->x, link->y, LINK);
            Nokia5110_DisplayBuffer();

            if(link->y < MAX_Y - 2) link->y+=2;
            else link->y = MAX_Y - 1;

            link->direction = DOWN;
            link->last_sprite = link->sprite[WALKING+link->step][link->direction];
            Nokia5110_PrintBMP(link->x,link->y,link->last_sprite,0);
            Nokia5110_DisplayBuffer();
            link->step = !(link->step);
            Level_WarMapUpdate(link, enemy, link->last_sprite, link->x, link->y, LINK);
            break;

        case LEFT:
            Nokia5110_ClearBitmap(link->x,link->y,link->last_sprite);
            Level_WarMapClear(link->last_sprite, link->x, link->y, LINK);
            Nokia5110_DisplayBuffer();

            if(link->x >= 2) link->x-=2;
            else link->x = 0;

            link->direction = LEFT;
            link->last_sprite = link->sprite[WALKING+link->step][link->direction];
            Nokia5110_PrintBMP(link->x,link->y,link->last_sprite,0);
            Nokia5110_DisplayBuffer();
            link->step = !(link->step);
            Level_WarMapUpdate(link, enemy, link->last_sprite, link->x, link->y, LINK);
            break;

        case SWORD:
            Nokia5110_ClearBitmap(link->x,link->y,link->last_sprite);

            switch(link->direction){
                case UP:
                    if(link->y < 24){
                        link->sword = 0;
                    }else if(link->y < 30){
                        link->sword = sword_half_sprite[UP];
                    }else{
                        link->sword = sword_sprite[UP];
                    }
                    break;
                case RIGHT:
                    if(link->x > 60){
                        link->sword = 0;
                    }else if(link->x > 53){
                        link->sword = sword_half_sprite[RIGHT];
                    }else{
                        link->sword = sword_sprite[RIGHT];
                    }
                    break;
                case DOWN:
                    if(link->y > 39){
                        link->sword = 0;
                    }else if(link->y > 32){
                        link->sword = sword_half_sprite[DOWN];
                        sword_position_y[DOWN] = 8;
                    }else{
                        link->sword = sword_sprite[DOWN];
                        sword_position_y[DOWN] = 15;
                    }
                    break;
                case LEFT:
                    if(link->x < 10){
                        link->sword = 0;
                    }else if(link->x < 16){
                        link->sword = sword_half_sprite[LEFT];
                        sword_position_x[LEFT] = -10;
                    }else{
                        link->sword = sword_sprite[LEFT];
                        sword_position_x[LEFT] = -16;
                    }
                    break;
            }
            Nokia5110_DisplayBuffer();

            // first we put the sword on the screen if it fits in
            Nokia5110_PrintBMP(link->x+sword_position_x[link->direction],link->y+sword_position_y[link->direction],link->sword,0);
            // then we make Link appear.
            // this way Link pixels overlap the sword making a best animation effect
            Nokia5110_PrintBMP(link->x,link->y,link->sprite[ATTACKING][link->direction],0);

            Nokia5110_DisplayBuffer();
            SysCtlDelay(SysCtlClockGet() / 25);

            // clear sword bitmap on the screen
            Nokia5110_ClearBitmap(link->x+sword_position_x[link->direction],link->y+sword_position_y[link->direction],link->sword);
            Nokia5110_PrintBMP(link->x,link->y,link->last_sprite,0);

            Nokia5110_DisplayBuffer();

            // update the warmap for the sword
            Level_WarMapUpdate(link,enemy,link->sword,link->x+sword_position_x[link->direction],link->y+sword_position_y[link->direction],SWORD);

            // clear the warmap for the sword
            Level_WarMapClear(link->sword,link->x+sword_position_x[link->direction],link->y+sword_position_y[link->direction],SWORD);

            break;
        case PAUSE:
            if(!Pause()){
                link->life=0;
                Link_IsDead(link);
            };
        default: break;
    }
}

// Set the hero to attack mode
void Link_Attack(Link_t *link, Enemy_t *enemy){

    Nokia5110_ClearBitmap(enemy->x,enemy->y,enemy->last_sprite);
    Nokia5110_DisplayBuffer();

    enemy->life--;
    if(!enemy->life){
        enemy->status = DUMB;
        link->enemies_to_kill--;
        survivor_points++;

        Nokia5110_ClearBitmap(enemy->x,enemy->y,enemy->last_sprite);
        Level_WarMapClear(enemy->last_sprite,enemy->x,enemy->y,ENEMY);

        uint8_t i;
        for(i=0;i<2;i++){
            enemy->last_sprite = enemy->sprite[enemy->step][ATTACKED2-i];
            Nokia5110_PrintBMP(enemy->x,enemy->y,enemy->last_sprite,0);
            Nokia5110_DisplayBuffer();
            SysCtlDelay(SysCtlClockGet()/10);
        }
        Nokia5110_ClearBitmap(enemy->x,enemy->y,enemy->last_sprite);
        Nokia5110_DisplayBuffer();
        enemy->last_sprite = 0;
    }
}

// Link loses life and change his position
void Link_IsAttacked(Link_t *link, Enemy_t *enemy){

        uint8_t forward = 3*enemy->status;

        Nokia5110_ClearBitmap(link->x,link->y,link->last_sprite);
        Nokia5110_ClearBitmap(enemy->x,enemy->y,enemy->last_sprite);
        Level_WarMapClear(link->last_sprite, link->x, link->y, LINK);
        Level_WarMapClear(enemy->last_sprite, enemy->x, enemy->y, ENEMY);

        // change Link's position based on his last direction

        if(enemy->status==DUMB){
            switch(link->direction){
                case UP:
                    link->y +=2;
                    break;
                case RIGHT:
                    link->x -=2;
                    break;
                case DOWN:
                    link->y -=2;
                    break;
                case LEFT:
                    link->x +=2;
                    break;
                default: break;
            }
        }else{
            switch(link->direction){
                case UP:
                    if(link->y < MAX_Y - 7) link->y+=6;
                    else link->y = MAX_Y -1;

                    if(enemy->y >= enemy->size_y + forward+1) enemy->y-=forward;
                    else enemy->y = enemy->size_y+1;

                    break;

                case RIGHT:
                    if(link->x >= 7) link->x-=6;
                    else link->x = 0;

                    if(enemy->x < MAX_X - enemy->size_x - forward-1) enemy->x+=forward;
                    else enemy->x = MAX_X -1;

                    break;

                case DOWN:
                    if(link->y >= link->size_y + 7) link->y-=6;
                    else link->y = link->size_y+1;

                    if(enemy->y < MAX_Y - forward-1) enemy->y+=forward;
                    else enemy->y = MAX_Y -1;

                    break;

                case LEFT:
                    if(link->x < MAX_X - link->size_x - 7) link->x+=6;
                    else link->x = MAX_X -1;

                    if(enemy->x >= forward+1) enemy->x-=forward;
                    else enemy->x = 0;

                    break;
            }
        }

        Nokia5110_PrintBMP(link->x,link->y,link->last_sprite,0);
        Nokia5110_PrintBMP(enemy->x,enemy->y,enemy->last_sprite,0);

        Nokia5110_DisplayBuffer();
        SysCtlDelay(SysCtlClockGet() / 20);

        Link_LifeLoss(link,enemy->damage);
        Level_WarMapUpdate(link, enemy, link->last_sprite, link->x, link->y, LINK);
        Level_WarMapUpdate(link, enemy, enemy->last_sprite, enemy->x, enemy->y, ENEMY);
}

// Link loses the same amount of life that the enemy's damage value
void Link_LifeLoss(Link_t *link, uint8_t damage){
    link->life -= damage;
    Lifebar_Update(link->life);
    if(link->life<=0) Link_IsDead(link);
}

// Game Over
void Link_IsDead(Link_t *link){
    Nokia5110_ClearBitmap(link->x,link->y,link->last_sprite);
    Nokia5110_PrintBMP(link->x,link->y,link_dead,0);
    Nokia5110_DisplayBuffer();
    SysCtlDelay(SysCtlClockGet()/5);
    Nokia5110_ClearBuffer();
    Nokia5110_Clear();
    SysCtlDelay(SysCtlClockGet()/5);
    Nokia5110_PrintBMP(4,26,GameOver3,0);
    Nokia5110_DisplayBuffer();
    SysCtlDelay(SysCtlClockGet()/15);
    Nokia5110_PrintBMP(4,26,GameOver2,0);
    Nokia5110_DisplayBuffer();
    SysCtlDelay(SysCtlClockGet()/15);
    Nokia5110_PrintBMP(4,26,GameOver,0);
    Nokia5110_DisplayBuffer();

    Nokia5110_PrintBMP(35,42,link_dead,0);
    Nokia5110_DisplayBuffer();
    while(GetSwitch(GetButton())==BUTTON_NOT_PRESSED){}

    Level_WarMapStart(warmap);

    Nokia5110_Clear();
    Nokia5110_ClearBuffer();
}

// =====================================================
// ### ENEMY ACTIONS ###

// Summon up a new enemy on a fixed place at the display

Enemy_t Enemy_New(uint8_t index, uint8_t x, uint8_t y, uint8_t life, uint8_t damage, uint8_t status){
    Enemy_t monster;
    monster.x = x;
    monster.y = y;
    monster.sprite[0] = enemy_array[index][0];
    monster.sprite[1] = enemy_array[index][1];
    monster.last_sprite = monster.sprite[0][0];
    monster.size_x = Nokia5110_getWidth(monster.last_sprite);
    monster.size_y = Nokia5110_getHeight(monster.last_sprite);
    monster.life = life;
    monster.damage = damage;

    monster.status = status;
    monster.step = 0;
    monster.direction = DOWN;

    Nokia5110_PrintBMP(monster.x, monster.y,monster.last_sprite,0);
    Nokia5110_DisplayBuffer();

    return monster;
}

// Change the enemy position and sprite
// Some enemies will follow Link, some have pattern moves
void Enemy_Move(Link_t *link, Enemy_t *enemy){
    uint8_t m; // monster index

    for(m=0;m<6;m++){

        Nokia5110_PrintBMP(enemy[m].x,enemy[m].y,enemy[m].last_sprite,0);
        Nokia5110_DisplayBuffer();

        if(enemy[m].status==FOLLOWER){
            Nokia5110_ClearBitmap(enemy[m].x,enemy[m].y,enemy[m].last_sprite);
            Level_WarMapClear(enemy[m].last_sprite, enemy[m].x, enemy[m].y, ENEMY);

            int sign;

            if(link->x < enemy[m].x) enemy[m].direction = LEFT;
            if(link->x > enemy[m].x) enemy[m].direction = RIGHT;

            sign = (link->x - enemy[m].x > 0) - (link->x - enemy[m].x < 0);
            enemy[m].x += sign;

            sign = (link->y - enemy[m].y > 0) - (link->y - enemy[m].y < 0);
            // this way big enemies don't go out of the screen
            if(enemy[m].y>=enemy[m].size_y) enemy[m].y += sign;

            // updates enemy last sprite
            enemy[m].last_sprite = enemy[m].sprite[enemy[m].step][enemy[m].direction];
            enemy[m].step = !(enemy[m].step); // alternate enemy step for sprite animation

            Nokia5110_PrintBMP(enemy[m].x,enemy[m].y,enemy[m].last_sprite,0);
            Nokia5110_DisplayBuffer();

            Level_WarMapUpdate(link, enemy, enemy[m].last_sprite, enemy[m].x, enemy[m].y, ENEMY);
        }


        if(enemy[m].status==ACTIVE){
            Nokia5110_ClearBitmap(enemy[m].x,enemy[m].y,enemy[m].last_sprite);
            Level_WarMapClear(enemy[m].last_sprite, enemy[m].x, enemy[m].y, ENEMY);
            srand(SysTickValueGet());

            enemy[m].direction = rand()%5;

            // Change enemy direction and move it randomly
            switch(enemy[m].direction){
                case UP:
                    enemy[m].direction = UP;
                    if(enemy[m].y > enemy[m].size_y + 2){
                        // change enemy position if it is not in the screen border
                        enemy[m].y-=2;
                    }else{
                        // if enemy go out of the screen, change it's position to the border
                        enemy[m].y = enemy[m].size_y;
                    }
                    break;

                case RIGHT:
                    enemy[m].direction = RIGHT;
                    if(enemy[m].x < MAX_X - enemy[m].size_x - 2){
                        enemy[m].x+=2;
                    }else{
                        enemy[m].x = MAX_X - enemy[m].size_x - 1;
                    }
                    break;

                case DOWN:
                    enemy[m].direction = DOWN;
                    if(enemy[m].y < MAX_Y - 2){
                        enemy[m].y+=2;
                    }else{
                        enemy[m].y = MAX_Y - 1;
                    }
                    break;

                case LEFT:
                    enemy[m].direction = LEFT;
                    if(enemy[m].x >= 2){
                        enemy[m].x-=2;
                    }else{
                        enemy[m].x = 0;
                    }
                    break;

                default:
                    enemy[m].direction = UP;
                    break;
            }
            // updates enemy last sprite
            enemy[m].last_sprite = enemy[m].sprite[enemy[m].step][enemy[m].direction];
            enemy[m].step = !(enemy[m].step); // alternate enemy step for sprite animation

            Nokia5110_PrintBMP(enemy[m].x,enemy[m].y,enemy[m].last_sprite,0);
            Nokia5110_DisplayBuffer();

            Level_WarMapUpdate(link, enemy, enemy[m].last_sprite, enemy[m].x, enemy[m].y, ENEMY);

        }

    }
}

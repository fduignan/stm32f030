#include "console.h"

void PlayMoonlander();
#define LEM1 COLOUR(255,64,64)
#define LEM2 COLOUR(255,0,0)
#define LEM3 COLOUR(255,255,255)
#define LEM4 COLOUR(128,128,255)
#define LEM5 COLOUR(255,255,255)
#define LEM6 COLOUR(255,255,0)
#define SPCE COLOUR(0,0,0)

// Image with engine off
const uint16_t LEMImage1[] = { 
    SPCE,SPCE,SPCE,SPCE,LEM5,LEM5,SPCE,SPCE,SPCE,SPCE, 
    SPCE,SPCE,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,SPCE,SPCE,
    SPCE,LEM5,LEM5,SPCE,SPCE,SPCE,SPCE,LEM5,LEM5,SPCE,
    SPCE,LEM5,SPCE,SPCE,SPCE,SPCE,SPCE,SPCE,LEM5,SPCE,
    SPCE,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,SPCE,
    SPCE,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,SPCE,
    SPCE,SPCE,SPCE,SPCE,LEM3,LEM3,SPCE,SPCE,SPCE,SPCE,
    SPCE,LEM3,LEM3,LEM3,LEM3,LEM3,LEM3,LEM3,LEM3,SPCE,
    SPCE,SPCE,LEM1,SPCE,LEM2,LEM2,SPCE,LEM1,SPCE,SPCE,
    SPCE,SPCE,LEM1,SPCE,LEM2,LEM2,SPCE,LEM1,SPCE,SPCE,
    LEM1,LEM1,SPCE,SPCE,SPCE,SPCE,SPCE,SPCE,LEM1,LEM1,
    SPCE,SPCE,SPCE,SPCE,SPCE,SPCE,SPCE,SPCE,SPCE,SPCE,    
    };   

// Image with engine on
const uint16_t LEMImage2[] = {
    SPCE,SPCE,SPCE,SPCE,LEM5,LEM5,SPCE,SPCE,SPCE,SPCE, 
    SPCE,SPCE,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,SPCE,SPCE,
    SPCE,LEM5,LEM5,SPCE,SPCE,SPCE,SPCE,LEM5,LEM5,SPCE,
    SPCE,LEM5,SPCE,SPCE,SPCE,SPCE,SPCE,SPCE,LEM5,SPCE,
    SPCE,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,SPCE,
    SPCE,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,LEM5,SPCE,
    SPCE,SPCE,SPCE,SPCE,LEM3,LEM3,SPCE,SPCE,SPCE,SPCE,
    SPCE,LEM3,LEM3,LEM3,LEM3,LEM3,LEM3,LEM3,LEM3,SPCE,
    SPCE,SPCE,LEM1,SPCE,LEM2,LEM2,SPCE,LEM1,SPCE,SPCE,
    SPCE,SPCE,LEM1,SPCE,LEM2,LEM2,SPCE,LEM1,SPCE,SPCE,
    SPCE,LEM1,SPCE,LEM2,LEM6,LEM6,LEM2,SPCE,LEM1,SPCE,
    SPCE,SPCE,SPCE,LEM6,LEM6,LEM6,LEM6,SPCE,SPCE,SPCE,    
};

// Initial test program for ST7789
#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "console.h"
#include "spi.h"
#include "timer.h"
#include "sprite.h"
#include "brick.h"
//#include "imagedata.h"
#include "invaders.h"
#include "logo_bricks.h"
void setup()
{
    // Turn on Port F
    RCC->AHBENR |= (1 << 22);
    // Make PF0 an output
    GPIOF->MODER |= 1;
    GPIOF->MODER &= ~(1 << 1);    
    // Turn on Port A
    RCC->AHBENR |= (1 << 17);        
    initSPI();
}

void initADC()
{
    RCC->APB2ENR |= (1 << 9); // enabled ADC clock  
    ADC->CR = 0;
    ADC->CHSELR = (1 << 16);
    ADC->CCR = (1 << 23) + (1 << 22); // enable reference and temp sensor
    
    ADC->CR |= (1 << 31); // start calibration
    while ( ADC->CR & (1 << 31) ); // wait for calibration to complete
    ADC->SMPR = 0x07; // longest S+H time
    ADC->CR = 1; // enable the ADC
}

void DrawLogo(uint16_t Colour)
{
    brick Logo[LOGO_BRICK_COUNT];
    int Count;
    volatile int BrickDataIndex = 0;    
    uint16_t x,y,w,h;
    uint16_t r,g,b;
    Console.fillRectangle(0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1,0);

// The logo is defined in the header file  logo_bricks.h
// The colours are defined in this file using just 2 bits for each colour component (r,g,b): total 6 bits of colour
// The following loop defines all of the bricks in memory and "expands" their colour definition to the 16 bits required by the display
    for (Count = 0; Count < LOGO_BRICK_COUNT; Count++)
    {        
        x = brickattribs[BrickDataIndex];
        y = brickattribs[BrickDataIndex + 1];
        w = brickattribs[BrickDataIndex + 2];
        h = brickattribs[BrickDataIndex + 3];
        Colour = brickattribs[BrickDataIndex + 4];
        r = (Colour & 0x30) >> 4;
        switch (r) {
            case 0: r=0;break;
            case 1: r=63;break;
            case 2: r=127;break;
            case 3: r=255;break;
        }
        g = (Colour & 0x0c) >> 2;
        switch (g) {
            case 0: g=0;break;
            case 1: g=63;break;
            case 2: g=127;break;
            case 3: g=255;break;
        }
        b = (Colour & 0x03);
        switch (b) {
            case 0: b=0;break;
            case 1: b=63;break;
            case 2: b=127;break;
            case 3: b=255;break;
        }
        Colour = COLOUR(r,g,b); // convert the 3 x 8 bit values to a single 16 bit value        
        Logo[Count].define(x,y,w,h,Colour);                           
        BrickDataIndex += 5;
        Logo[Count].show();        
    }
/* Dublin breakout */
    brick Bat(0, SCREEN_HEIGHT-20, SCREEN_WIDTH, 3,COLOUR(255,255,255));    
    brick Ball(120, SCREEN_HEIGHT/2, 3, 3,COLOUR(255,255,255));
#define MAX_BRICI_LEVELS 4
    int Level = MAX_BRICI_LEVELS;
    int LevelComplete = 0;
    int8_t BallCount = 5;
    uint8_t Index;
    int8_t BallXVelocity = 1;
    int8_t BallYVelocity = 1;    
    while (Level > 0)
    {
        Ball.move(Console.random(10, SCREEN_WIDTH-10), SCREEN_HEIGHT/2);
        if (Console.random(0,2) > 0)
            BallXVelocity = 1;
        else
            BallXVelocity = -1;
        LevelComplete = 0;
        BallYVelocity = -1;        
        for (Index = BallCount; Index > 0; Index--)
            Console.fillRectangle(SCREEN_WIDTH - Index * 15, SCREEN_HEIGHT-10, 10, 10, RGBToWord(0xff, 0xf, 0xf));        
        Ball.show();
        Bat.show();
        Console.print("Level", 5, 5, SCREEN_HEIGHT-10, RGBToWord(0xff, 0xff, 0xff), RGBToWord(0, 0, 0));
        Console.print(MAX_BRICI_LEVELS - Level + 1, 60, SCREEN_HEIGHT-10, RGBToWord(0xff, 0xff, 0xff), RGBToWord(0, 0, 0));
        
        while (!LevelComplete)
        {            
            if (Console.Controller.getButtonState() & Console.Controller.Right)
            {
                // Move right
                if (Bat.getX() < (SCREEN_WIDTH - Bat.getWidth()))
                {
                    Bat.move(Bat.getX() + 2, Bat.getY()); // Move the bat faster than the ball
                }
            }
            
            if (Console.Controller.getButtonState() & Console.Controller.Left)
            {
                // Move left
                if (Bat.getX() > 0)
                {
                    Bat.move(Bat.getX() - 2, Bat.getY()); // Move the bat faster than the ball
                }
            }
            
            if (Bat.touching(Ball.getX() + Ball.getWidth() / 2, Ball.getY() + Ball.getHeight() / 2))
            { // Ball bounced off the bat
                BallYVelocity = -BallYVelocity; 
                Console.Sound.playTone(200,20);
            }                        
            Ball.move(Ball.getX() + BallXVelocity, Ball.getY() + BallYVelocity); // update ball position
            if (Ball.getX() == 2) // bounced off left side?
            {
                BallXVelocity = -BallXVelocity;
                Console.Sound.playTone(400,20);
            }
            if (Ball.getX() == SCREEN_WIDTH - 2) // bounced off right side?
            {
                BallXVelocity = -BallXVelocity;
                Console.Sound.playTone(400,20);
            }
            if (Ball.getY() == 2)  // bounced off top? (behind the block)
            {
                BallYVelocity = -BallYVelocity;
                Console.Sound.playTone(400,20);
            }
            
            if (Ball.getY() >= Bat.getY() + Bat.getHeight() + 2)  // Did the ball go behind the bat?
            {
                BallCount--;  // Lost a ball!
                if (BallCount == 0) // Any left?
                {
                    // Nope: hard luck, game over
                    Console.fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
                    Console.print("GAME OVER", 9, 40, 100, RGBToWord(0xff, 0xff, 0xff), 0);
                    Console.print("Fire to restart", 15, 8, 120, RGBToWord(0xff, 0xff, 0), RGBToWord(0, 0, 0));
                    while (!(Console.Controller.getButtonState() & Console.Controller.Fire))
                        Console.Timer.sleep(10);
                    return;
                }
                // start a new ball moving in a random way
                if (Console.random(0,100) & 1)
                    BallXVelocity = 1;
                else
                    BallXVelocity = -1;
                BallYVelocity = -1;
                Ball.move(Console.random(10, SCREEN_WIDTH - 10), Console.random(90, 120));
                Console.fillRectangle(SCREEN_WIDTH-5*15, SCREEN_HEIGHT-10, 120, 10, 0);  // wipe out the remaining lives area
                for (Index = BallCount; Index > 0; Index--) //  draw remaining lives
                    Console.fillRectangle(SCREEN_WIDTH - Index * 15, SCREEN_HEIGHT-10, 10, 10, RGBToWord(0xff, 0xf, 0xf));
                
            }
            LevelComplete = 1;
            // Check to see if the ball has hit any of the blocks
            for (Index = 0; Index < LOGO_BRICK_COUNT; Index++)
            {
                if (Logo[Index].visible()) // any blocks left?
                    LevelComplete = 0; // If yes then the level is not complete
                    
                int touch = 0;
                // Check all 4 corners of the ball for touching
                touch += Logo[Index].touching(Ball.getX(), Ball.getY());
                if (touch == 0)
                    touch = Logo[Index].touching(Ball.getX()+2, Ball.getY());
                if (touch == 0)
                    touch = Logo[Index].touching(Ball.getX(), Ball.getY()+2);
                if (touch == 0)
                    touch = Logo[Index].touching(Ball.getX()+2, Ball.getY()+2);
                if (touch)
                { // Block hit so hide it.
                    Logo[Index].hide();  
                    if ( (touch == 1) || (touch == 3) )
                        BallYVelocity = -BallYVelocity;
                    if ( (touch == 2) || (touch == 4) )
                        BallXVelocity = -BallXVelocity;
                    Console.Sound.playTone(1000,20);
                }
            }
            // No Blocks left, Move to next level.
            if ((LevelComplete == 1) && (Level > 0))
            {
                Level--;
                Ball.hide();
                Console.print("Level", 5, 5, SCREEN_HEIGHT-10, RGBToWord(0xff, 0xff, 0xff), RGBToWord(0, 0, 0));
                Console.print(MAX_BRICI_LEVELS - Level + 1, 60, SCREEN_HEIGHT-10, RGBToWord(0xff, 0xff, 0xff), RGBToWord(0, 0, 0));
                
            }
            Console.Timer.sleep(Level+5); // Slow the game to human speed
            Bat.redraw(); // redraw bat as it might have lost a pixel due to collisions
          //  Console.Timer.sleep(10); // Slow the game to human speed
        }
    }
}
void debugScreen()
{
    while(1)
    {
        
        Console.fillRectangle(0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1,0);
        Console.print("Button state :",14,0,10,RGBToWord(0xff,0xff,0xff),0);
        Console.print(Console.Controller.getButtonState(),130,10,RGBToWord(0xff,0xff,0xff),0);
        Console.print("Rand: ",6,0,20,RGBToWord(0xff,0xff,0xff),0);
        Console.print(Console.random(0,100),130,20,RGBToWord(0xff,0xff,0xff),0);
        uint8_t Pkt[4];
        Pkt[0]=0xaa;
        Pkt[1]=0x55;
        Console.Ibc.sendPacket(0xfe,0,2,Pkt);
        Console.Ibc.sendPacket(0xfd,0,2,Pkt);
        Console.Sound.playTone(1000,200); 
        Console.Timer.sleep(100);        
    }
}

int main()
{        
    int Count=0;
    setup();
    Console.begin();       
    
    // Use GPIOA bit 3 for debugging 
    GPIOA->MODER |= (1 << 6);
    GPIOA->MODER &= ~(1 << 7);
    int x,y;
    //if (Console.Controller.getButtonState()==(Console.Controller.Left+Console.Controller.Right))
    {
        debugScreen();
    }
    
    while(1)
    {            
		for (Count = 0; Count < 20; Count++)
		{
			DrawLogo(0xffff);
		}		
		Console.fillRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT-1,0);     
        for (Count=0;Count < 200; Count++)
        {
            Console.fillCircle(Console.random(1,190),Console.random(50,190),Console.random(10,40),Console.random(10,65535));
            Console.print("Dublin Maker",12,70,10,RGBToWord(0xff,0xff,0xff),0);
            Console.print("2019",4,110,20,RGBToWord(0xff,0xff,0xff),0);            

        }
        Console.fillRectangle(0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1,0);
        for (Count=0;Count < 200; Count++)
        {
            Console.fillRectangle(Console.random(1,199),Console.random(40,199),Console.random(10,40),Console.random(10,40),Console.random(10,65535));
            Console.print("Dublin Maker",12,70,10,RGBToWord(0xff,0xff,0xff),0);
            Console.print("2019",4,110,20,RGBToWord(0xff,0xff,0xff),0);            

        }              
        Console.fillRectangle(0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1,0);
        for (Count=0;Count < 200; Count++)
        {
            Console.drawCircle(Console.random(1,190),Console.random(50,190),Console.random(10,40),Console.random(10,65535));
            Console.print("Dublin Maker",12,70,10,RGBToWord(0xff,0xff,0xff),0);            
            Console.print("2019",4,110,20,RGBToWord(0xff,0xff,0xff),0);            

        }
        Console.fillRectangle(0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1,0);
        for (Count=0;Count < 200; Count++)
        {
            Console.drawRectangle(Console.random(1,199),Console.random(40,199),Console.random(10,40),Console.random(10,40),Console.random(10,65535));
            Console.print("Dublin Maker",12,70,10,RGBToWord(0xff,0xff,0xff),0);
            Console.print("2019",4,110,20,RGBToWord(0xff,0xff,0xff),0);            

        }              
        Console.fillRectangle(0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1,0);
        Console.print("Dublin Maker",12,70,10,RGBToWord(0xff,0xff,0xff),0);
        Console.print("2019",4,110,20,RGBToWord(0xff,0xff,0xff),0);        
        
        Console.fillRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT-1,0);                        
        playInvaders();
    }
}

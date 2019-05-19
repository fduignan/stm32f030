// Initial test program for ST7789
#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "console.h"
#include "spi.h"
#include "timer.h"
#include "sprite.h"
#include "brick.h"
#include "brici.h"
//#include "imagedata.h"
#include "invaders.h"
#include "logo_bricks.h"
#include "tennis.h"
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



void DrawLogo(uint32_t demo_mode)
{
    brick Logo[LOGO_BRICK_COUNT];
    int Count;
    volatile int BrickDataIndex = 0;    
    uint16_t x,y,w,h;
    uint16_t r,g,b;
    uint16_t Colour;
    uint16_t Brick_Count = LOGO_BRICK_COUNT;
 // The logo is defined in the header file  logo_bricks.h
// The colours are defined in this file using just 2 bits for each colour component (r,g,b): total 6 bits of colour
// The following loop defines all of the bricks in memory and "expands" their colour definition to the 16 bits required by the display
    Console.fillRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,0);
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
        
    }   
    if (demo_mode == 0)
    {
        for (Count = 0; Count < LOGO_BRICK_COUNT; Count++)
        {
            Logo[Count].show();        
        }

    }
    if (demo_mode==1)
    {
        for (Count = 0; Count < LOGO_BRICK_COUNT; Count++)
        {
            Logo[Count].show();        
        }
        Brici(Logo,Brick_Count,SCREEN_WIDTH,1);
    }
    if (demo_mode==2)
    {
        sprite Invader = { DefaultInvaderImage, 3, 3, 10, 12};
        sprite AttackerMissile = { AttackerMissileImage, 0, 0, 5, 8  };
        Invader.show();
        Invader.move(Invader.getX()+Console.random(0,200),Invader.getY());
        int XDirectionChange = 0;
        int YDirectionChange = 0;
        int x_step = 2;
        int y_step = 1;
        for (Count = 0; Count < LOGO_BRICK_COUNT; Count++)
        {
            Logo[Count].move(Logo[Count].getX(),Logo[Count].getY()+120);
            Logo[Count].show();                    
        }
       
        for (Count = 0; Count < 4000; Count++)
        {
            if (Console.Controller.getButtonState()) // exit demo if the user presses a button
                return;
            XDirectionChange=0;
            YDirectionChange=0;
            Invader.move(Invader.getX() + x_step, Invader.getY() + y_step);
            if (Invader.getX() >= (SCREEN_WIDTH - Invader.getWidth()))
                XDirectionChange = 1;
            if (Invader.getX() == 0)
                XDirectionChange = 1;
            if (Invader.getY() > SCREEN_HEIGHT - 140)
            {
                YDirectionChange = 1;
                if (Invader.getY() < 3)
                {
                    YDirectionChange = 1;
                }
            }
            if (XDirectionChange) // Did an invader hit either edge?
                x_step = -x_step; // if so, then reverse direction
            if (YDirectionChange) // Did an invader hit either edge?
                y_step = -y_step; // if so, then reverse direction
            if (AttackerMissile.visible())
            {
                int16_t MissileTipX,MissileTipY;
                MissileTipX = AttackerMissile.getX()+AttackerMissile.getWidth() / 2;
                MissileTipY = AttackerMissile.getY()+AttackerMissile.getHeight() /2;
                for (int Index = 0; Index < LOGO_BRICK_COUNT; Index++)
                {
                    if (Logo[Index].visible())
                    {
                        if (Logo[Index].within(MissileTipX,MissileTipY))
                        {
                            Logo[Index].hide();
                            AttackerMissile.hide();
                           // Console.Sound.playTone(400,20);
                        }
                    }                                        
                }
                AttackerMissile.move(AttackerMissile.getX(),AttackerMissile.getY()+2);
                if (AttackerMissile.getY() > SCREEN_HEIGHT)
                    AttackerMissile.hide();
            }
            else
            {
                AttackerMissile.show();
                AttackerMissile.move(Invader.getX() + Invader.getWidth() / 2 - AttackerMissile.getWidth() / 2, Invader.getY() + AttackerMissile.getWidth() / 2);
            }
            Console.Timer.sleep(10);
            
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
        Console.Sound.playTone(500,200); 
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
    if (Console.Controller.getButtonState()==(Console.Controller.Fire+Console.Controller.Right))
    {
        debugScreen();
    }
    Tennis();
    while (1) 
    {
        // Present a menu to the user
        DrawLogo(0);        
        Console.print("Press Fire for Brici",sizeof("Press Fire for Brici")-1,20,140,RGBToWord(0xff,0xff,0xff),0);
        Console.print("Press Left for Invaders",sizeof("Press Left for Invaders")-1,20,155,RGBToWord(0xff,0xff,0xff),0);
        Console.print("Press Right for Tennis",sizeof("Press Right for Tennis")-1,20,170,RGBToWord(0xff,0xff,0xff),0);
        uint32_t choice=0;
        uint32_t Timeout=200;
        do {
            choice = Console.Controller.getButtonState();
            Console.Timer.sleep(100);
        }
        while ( (choice == 0) && (Timeout--) );
        switch (choice)
        {
            case Console.Controller.Fire : {
                 ClassicBrici();
                 break;
            }
            case Console.Controller.Left : {
                 playInvaders(0);
                 break;
            }
            case Console.Controller.Right : {
                 Tennis();
                 break;
            }
            default: {
                while(Console.Controller.getButtonState()==0)
                {            
                    DrawLogo(1);
                    Console.fillRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT-1,0);             
                    DrawLogo(2);
                    Console.Timer.sleep(1000);                    
                    Console.fillRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT-1,0);                                                  
                }
                break;
            }
        }
                                
    }
}

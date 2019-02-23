// Initial test program for ST7789
#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "console.h"
#include "spi.h"
#include "timer.h"
#include "sprite.h"

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
#define BGND RGBToWord(0,0,0)
#define IVDR RGBToWord(0x00,0x00,0xff)
const uint16_t DefaultInvaderImage[] = { 
    BGND,BGND,BGND,BGND,BGND,BGND,BGND,BGND,BGND,BGND,
    BGND,BGND,BGND,BGND,IVDR,IVDR,BGND,BGND,BGND,BGND,
    BGND,BGND,BGND,IVDR,IVDR,IVDR,IVDR,BGND,BGND,BGND,
    BGND,IVDR,IVDR,BGND,IVDR,IVDR,BGND,IVDR,IVDR,BGND,
    BGND,IVDR,IVDR,BGND,IVDR,IVDR,BGND,IVDR,IVDR,BGND,
    BGND,IVDR,IVDR,IVDR,IVDR,IVDR,IVDR,IVDR,IVDR,BGND,
    BGND,IVDR,IVDR,IVDR,IVDR,IVDR,IVDR,IVDR,IVDR,BGND,
    BGND,BGND,IVDR,IVDR,BGND,BGND,IVDR,IVDR,BGND,BGND,
    BGND,BGND,IVDR,IVDR,BGND,BGND,IVDR,IVDR,BGND,BGND,
    BGND,IVDR,IVDR,BGND,BGND,BGND,BGND,IVDR,IVDR,BGND,
    IVDR,IVDR,IVDR,BGND,BGND,BGND,BGND,IVDR,IVDR,IVDR,
    BGND,BGND,BGND,BGND,BGND,BGND,BGND,BGND,BGND,BGND 
    }; 
sprite Invader(DefaultInvaderImage,120,120,10,12);
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
uint16_t readTemperature()
{
    
    return Console.Controller.readTemperature();
}
int main()
{
    int Count=0;
    setup();
    Console.begin();       
    int x,y;
    while(1)
    {     
        for (Count=0;Count < 50; Count++)
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
        Invader.move(0,0);
        Invader.show();
        for (x=0;x<SCREEN_WIDTH;x++)
        {               
            Invader.move(x,x);
            Console.Timer.sleep(10);
        }              
        
        Invader.hide();
        Console.fillRectangle(0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1,0);                        
        
    }
}

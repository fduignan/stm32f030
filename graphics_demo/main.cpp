// Initial test program for ST7789
#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "console.h"
#include "spi.h"
#include "timer.h"
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





int main()
{        
    uint8_t IDData[4];
    int Count=0;
    setup();
    Console.begin();       
    

    while(0)
    {
         Console.Display.readDisplayID(IDData);
         Console.Timer.sleep(2);
    }
    while (1) 
    {
        // Draw some random rectangles
        for (Count = 0; Count < 200; Count++)
        {
            Console.drawRectangle(Console.random(0,239),Console.random(0,239),Console.random(0,120),Console.random(0,120),Console.random(0,0xffff));
        //    Console.Timer.sleep(50);                        
        }
        Console.fillRectangle(0,0,239,239,0);
        for (Count = 0; Count < 100; Count++)
        {
            Console.fillRectangle(Console.random(0,239),Console.random(0,239),Console.random(0,120),Console.random(0,120),Console.random(0,0xffff));
         //   Console.Timer.sleep(50);                        
        }
        Console.fillRectangle(0,0,239,239,0);
        for (Count = 0; Count < 200; Count++)
        {
            Console.drawCircle(Console.random(0,239),Console.random(0,239),Console.random(0,120),Console.random(0,0xffff));
         //   Console.Timer.sleep(50);                        
        }
        Console.fillRectangle(0,0,239,239,0);
        for (Count = 0; Count < 50; Count++)
        {
            Console.fillCircle(Console.random(0,239),Console.random(0,239),Console.random(0,120),Console.random(0,0xffff));
         //   Console.Timer.sleep(50);                        
        }
        Console.fillRectangle(0,0,239,239,0);
        
    }
}

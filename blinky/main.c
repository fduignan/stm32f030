#include <stdint.h>
#include "../include/STM32F0x0.h"
void delay(uint32_t dly)
{
    while(dly--);
}
void setup()
{
    // Turn on Port F
    RCC->AHBENR |= (1 << 22);
    // Make PF0 an output
    GPIOF->MODER |= 1;
    GPIOF->MODER &= ~(1 << 1);    
}
int main()
{
    setup();    
    while(1)
    {
        GPIOF->ODR |= 1; // led on
        delay(1000000);
        GPIOF->ODR &= ~1;// led off
        delay(1000000);        
    }
}

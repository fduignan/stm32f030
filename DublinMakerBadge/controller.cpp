#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "controller.h" 
/* 
 * Button configuratoin (Version 0.1)
 * PF0, PF1, PA0, PA1, PA2: Mapped to bits 0 to 4 in return of getButtonState respectively
 */
void controller::begin()
{
    RCC->APB2ENR |= (1 << 9); // enable ADC clock  
    ADC->CFGR2 |= (1 << 31); // 
    ADC->CFGR2 &= ~(1 << 30); // ADC Clock = PCLK/4
    ADC->CR = 0;
    ADC->CHSELR = (1 << 16);
    ADC->CCR = (1 << 23) + (1 << 22); // enable reference and temp sensor
    
    ADC->CR |= (1 << 31); // start calibration
    while ( ADC->CR & (1 << 31) ); // wait for calibration to complete
    ADC->SMPR = 0x07; // longest S+H time
    ADC->CR = 1; // enable the ADC
    // Turn on port F
    RCC->AHBENR |= (1 << 22);
    // Turn on port A 
    RCC->AHBENR |= (1 << 17);
    // Configure pins as inputs
    GPIOF->MODER &= ~(0x0f); // PF0 bits 0 and 1 are inputs
    GPIOA->MODER &= ~(0x3f); // PA0 bits 0,1,2 are inputs    
    // Enable pull-ups on button inputs
    GPIOF->PUPDR &= ~(0x0f);
    GPIOF->PUPDR |= (1 << 2) + (1 << 0);
    GPIOA->PUPDR &= ~(0x3f);
    GPIOA->PUPDR |= (1 << 4) + (1 << 2) + (1 << 0);
}
void controller::stopADC()
{
    ADC->CR = 0;
    ADC->CCR = 0;
    RCC->APB2ENR &= ~(1 << 9); // disable ADC clock  
}
uint16_t controller::getButtonState()
{
    uint16_t ReturnValue = 0;
    if (( GPIOF->IDR & (1 << 0) )==0)
    {
        ReturnValue |= 1<<0;
    }
    if (( GPIOF->IDR & (1 << 1) )==0)
    {
        ReturnValue |= 1<<1;
    }
    if (( GPIOA->IDR & (1 << 0) )==0)
    {
        ReturnValue |= 1<<2;
    }
    if (( GPIOA->IDR & (1 << 1) )==0)
    {
        ReturnValue |= 1<<3;
    }
    if (( GPIOA->IDR & (1 << 2) )==0)
    {
        ReturnValue |= 1<<4;
    }
    return ReturnValue;
}
uint16_t controller::readTemperature()
{
        
    
    ADC->CHSELR = (1 << 16);
    ADC->CR |= 1+(1 << 2); // start a conversion
    while((ADC->CR & (1 << 2))==0); // wait for EOC
    return ADC->DR;
}

#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "controller.h"    
void controller::begin()
{
    RCC->APB2ENR |= (1 << 9); // enable ADC clock  
    ADC->CR = 0;
    ADC->CHSELR = (1 << 16);
    ADC->CCR = (1 << 23) + (1 << 22); // enable reference and temp sensor
    
    ADC->CR |= (1 << 31); // start calibration
    while ( ADC->CR & (1 << 31) ); // wait for calibration to complete
    ADC->SMPR = 0x07; // longest S+H time
    ADC->CR = 1; // enable the ADC
}
void controller::stopADC()
{
    ADC->CR = 0;
    ADC->CCR = 0;
    RCC->APB2ENR &= ~(1 << 9); // disable ADC clock  
}
uint16_t controller::getButtonState()
{
    return 0;
}
uint16_t controller::readTemperature()
{
        
    ADC->CHSELR = (1 << 16);
    ADC->CR |= (1 << 2); // start a conversion
    while((ADC->CR & (1 << 2))); // wait for EOC
    return ADC->DR;
}

// spi.h for stm32f030
// Author: Frank Duignan.  
// Updates posted on http://ioprog.com 

#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "spi.h"
void initSPI(void)
{
	int drain_count,drain;
	
	RCC->APB2ENR |= (1 << 12);		// turn on SPI1 	
	
	
	// GPIOA bits 5 and 7 are used for SPI1 (Alternative functions 0)
	RCC->AHBENR |= (1 << 18)+(1 << 17); 	// enable ports A and B
    GPIOA->MODER &= ~( (1 << 14)+(1 << 10)); // select Alternative function
    GPIOA->MODER |= ((1 << 15)+(1 << 11));  // for bits 5,7 (not using MISO)
    GPIOA->AFRL &= 0x0f0fffff;		     // select Alt. Function 0
	
	// set port bits up as high speed outputs
	GPIOA->OSPEEDR |= (1 << 15) + (1 << 14) + (1 << 11) + (1 << 10);
	// Now configure the SPI interface
	drain = SPI1->SR;				// dummy read of SR to clear MODF	
	// enable SSM, set SSI, enable SPI, PCLK/2, LSB First Master, Clock = 1 when idle
	SPI1->CR1 = (1 << 9)+(1 << 8)+(1 << 6)+(1 << 2)+(1 << 1);	
	SPI1->CR2 = (1 << 10)+(1 << 9)+(1 << 8); 	// configure for 8 bit operation
   
    for (drain_count = 0; drain_count < 32; drain_count++)
		drain = transferSPI8(0x00);
}

uint8_t transferSPI8(uint8_t data)
{
    unsigned Timeout = 1000000;
    uint8_t ReturnValue;
    uint8_t *preg=(uint8_t*)&SPI1->DR;
	
    while (((SPI1->SR & (1 << 7))!=0)&&(Timeout--));
    *preg = data;
    Timeout = 1000000;
    while (((SPI1->SR & (1 << 7))!=0)&&(Timeout--));        
	ReturnValue = *preg;
	
    return ReturnValue;
}

uint16_t transferSPI16(uint16_t data)
{
    unsigned Timeout = 1000000;
    uint16_t ReturnValue;    
	
    while (((SPI1->SR & (1 << 7))!=0)&&(Timeout--));
    SPI1->DR = data;
    Timeout = 1000000;
    while (((SPI1->SR & (1 << 7))!=0)&&(Timeout--));        
	ReturnValue = SPI1->DR;
	
    return ReturnValue;
}

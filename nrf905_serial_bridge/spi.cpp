#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "spi.h"
int spi::begin()
{
    volatile int drain_count,drain;
	
	RCC->APB2ENR |= (1 << 12);		// turn on SPI1 	
	
	
	// GPIOA bits 5,6,7 are used for SPI1 (Alternative functions 0)
	RCC->AHBENR |= (1 << 18)+(1 << 17); 	// enable ports A and B
    GPIOA->MODER &= ~((1 << 14)+(1 << 12)+(1 << 10)); // select Alternative function
    GPIOA->MODER |= ((1 << 15)+(1 << 13)+(1 << 11));  // for bits 5,6,7
    GPIOA->AFRL &= 0x000fffff;		     // select Alt. Function 0
	
	// Now configure the SPI interface
	drain = SPI1->SR;				     // dummy read of SR to clear MODF
	// enable SSM, set SSI, enable SPI, PCLK/2, LSB First Master	
	SPI1->CR1 = (1 << 9)+(1 << 8)+(1 << 6)+ (1 << 2) + (1 << 5); // Slowed down to allow my anayzer read the bus	
	SPI1->CR2 = (1 << 12) + (1 << 10) + (1 << 9) + (1 << 8); 	     // configure for 8 bit operation, RXNE on byte received
    drain = SPI1->CR1;
      
    for (drain_count = 0; drain_count < 32; drain_count++)
		drain = SPI1->DR;
    drain = SPI1->SR;
    return 0;
}
uint8_t spi::transferSPI(uint8_t data)
{
    // Need to fight with the header file that was auto-generated from the SVD file
    // The data registers behave differently depending upon whether you do an 8 bit
    // or larget read/write.  The header file only defines an 32 bit data register 
    // so pointers and type casting is necessary to force the 8-bit read/write
    volatile unsigned Timeout = 1000000;    
    int status;
    int ReturnValue;
    uint8_t *pDR = (uint8_t *)&SPI1->DR;
    Timeout = 500;        
    *pDR = data; // Do 8-bit write
    while (((SPI1->SR & (1))==0)&&(Timeout--));  
    ReturnValue = *pDR;
    return ReturnValue;    
}

uint8_t spi::transferSPI(uint16_t data)
{
    volatile unsigned Timeout = 1000000;
    
    int status;
    int ReturnValue;
    
    Timeout = 500;            
    SPI1->DR = data;
    while (((SPI1->SR & (1))==0)&&(Timeout--));  
    status=SPI1->DR;        
    Timeout = 500;                  
    while (((SPI1->SR & (1))==0)&&(Timeout--));                
	ReturnValue = SPI1->DR;    
    return ReturnValue;    
}

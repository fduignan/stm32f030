// Initial test program for ST7789
#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "spi.h"
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
    // Turn on Port A
    RCC->AHBENR |= (1 << 17);        
}
void writeCommand(uint8_t cmd)
{
    GPIOA->ODR &= ~(1 << 10); // drive command pin low
    transferSPI(cmd);
}
void writeData(uint8_t data)
{
    GPIOA->ODR |= (1 << 10); // drive command pin high
    transferSPI(data);
}
void writeData16(uint16_t data)
{
    GPIOA->ODR |= (1 << 10); // drive command pin high
    transferSPI16(data);
}
void initDisplay()
{
    // Reset pin is on PA9
    GPIOA->MODER |= (1 << 18);
    GPIOA->MODER &= ~(1 << 19);
    // D/C pin is on PA10
    GPIOA->MODER |= (1 << 20);
    GPIOA->MODER &= ~(1 << 21);

    
    GPIOA->ODR |= (1 << 9); // Drive reset high
    delay(1000000); // wait   
    GPIOA->ODR &= ~(1 << 9); // Drive reset low
    delay(1000000); // wait     
    GPIOA->ODR |= (1 << 9); // Drive reset high
    delay(1000000); // wait     

    
    writeCommand(0x1);  // software reset
    delay(1000000); // wait     
    
    writeCommand(0x11);  //exit SLEEP mode
    delay(5000000); // wait     
            
    writeCommand(0x3A); // Set col mode        
    writeData(0x55); // 16bits / pixel @ 64k colors
    delay(1000000); // wait     
        
    writeCommand(0x36);
    writeData(0x00);  // RGB Format
    delay(1000000); // wait     


    writeCommand(0x51); // maximum brightness
    writeData(0xff); 
    
  //////// WINDOW SET ///////////////////////////////////////////
  writeCommand(0x2A);
  writeData(0x00);                  //column address set
  writeData(0x00);                  //start 0x0000
  writeData(0x00);
  writeData(0xef);                  //end 0x00EF

  writeCommand(0x2B);
  writeData(0x00);                  //page address set
  writeData(0x00);                  //start 0x0000
  writeData(0x00);
  writeData(0xef);                  //end 0x00EF

  delay(1000000); // wait     

  writeCommand(0x21);                 
  delay(1000000);
  
  writeCommand(0x13);                 
  delay(1000000);
    
  writeCommand(0x29);                 
  delay(1000000);
  writeCommand( 0x2c);   // memory write
  int i=0;
  // Cycle through some colours
  while(1)
  {
      
    for (i=0;i<240*240;i++)
    {
        writeData16(0xffff);        
    }    
    for (i=0;i<240*240;i++)
    {
        writeData16(0xf800);        
    }
    for (i=0;i<240*240;i++)
    {
        writeData16(0x7e0);        
    }
    for (i=0;i<240*240;i++)
    {
        writeData16(0x001f);        
    }
  }
    
}
int main()
{
    setup();
    initSPI();
    initDisplay();
    while(1)
    {
        GPIOF->ODR |= 1; // led on
        delay(1000000);
        GPIOF->ODR &= ~1;// led off
        delay(1000000);       
        
    }
}

#include <stdint.h>
#include "display.h"
#include "spi.h"
#include "../include/STM32F0x0.h"

void display::begin(timer &t)
{
    
    // Turn on Port A 
    RCC->AHBENR |= (1 << 17);     
    // Reset pin is on PA6
    GPIOA->MODER |= (1 << 12);
    GPIOA->MODER &= ~(1 << 13);
    // D/C pin is on PB1
    // Turn on Port B
    RCC->AHBENR |= (1 << 18);     
    GPIOB->MODER |= (1 << 2);
    GPIOB->MODER &= ~(1 << 3);
    
    
    RSTHigh(); // Drive reset high
    t.sleep(25); // wait   
    RSTLow(); // Drive reset low
    t.sleep(25); // wait   
    RSTHigh(); // Drive reset high
    t.sleep(25); // wait    
    
    
    writeCommand(0x1);  // software reset
    t.sleep(150); // wait   
    
    writeCommand(0x11);  //exit SLEEP mode
    t.sleep(25); // wait   
    
    writeCommand(0x3A); // Set colour mode        
    writeData8(0x55); // 16bits / pixel @ 64k colors
    t.sleep(25); // wait   
    
    writeCommand(0x36);
    writeData8(0x00);  // RGB Format
    t.sleep(25); // wait   
    
    
    writeCommand(0x51); // maximum brightness
    t.sleep(25); // wait   
    
    writeCommand(0x21);    // display inversion off (datasheet is incorrect on this point)
    writeCommand(0x13);    // partial mode off                 
    writeCommand(0x29);    // display on
    t.sleep(25); // wait   
    writeCommand( 0x2c);   // put display in to write mode
    fillRectangle(0,0,SCREEN_WIDTH, SCREEN_HEIGHT, 0x00);  // black out the screen
}
void display::putPixel(uint16_t x, uint16_t y, uint16_t colour)
{
    openAperture(x, y, x + 1, y + 1);
    writeData16(colour); 
}
void display::putImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *Image)
{
    uint16_t Colour;
    openAperture(x, y, x + width - 1, y + height - 1);
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            Colour = *(Image++);
            writeData16(Colour);
        }
    }
}
void display::fillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t Colour)
{
    openAperture(x, y, x + width - 1, y + height - 1);
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            writeData16(Colour);
        }
    }
}
void display::openAperture(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    // open up an area for drawing on the display
    writeCommand(0x2A); // Set X limits
    writeData8(x1>>8);
    writeData8(x1&0xff);    
    writeData8(x2>>8);
    writeData8(x2&0xff);
    writeCommand(0x2B); // Set Y limits
    writeData8(y1>>8);
    writeData8(y1&0xff);    
    writeData8(y2>>8);
    writeData8(y2&0xff);
    
    writeCommand(0x2c); // put display in to data write mode
}
void display::RSLow()
{
    GPIOB->ODR &= ~(1 << 1); // drive D/C pin low
}
void display::RSHigh()
{ 
    GPIOB->ODR |= (1 << 1); // drive D/C pin high
}
void display::RSTLow()
{
    GPIOA->ODR &= ~(1 << 6); // Drive reset low
}
void display::RSTHigh()
{
    GPIOA->ODR |= (1 << 6); // Drive reset high
}
void display::writeCommand(uint8_t Cmd)
{
    RSLow();
    transferSPI8(Cmd);
}
void display::writeData8(uint8_t Data)
{
    RSHigh();
    transferSPI8(Data);
}
void display::writeData16(uint16_t Data)
{
    RSHigh();
    transferSPI16(Data);
}

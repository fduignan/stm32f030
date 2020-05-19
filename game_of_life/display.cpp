#include "display.h"
#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "font5x7.h"
//display.cpp
/* Wiring:
 * PA0-PA7 = DB0-DB7
 * PB1 = Reset
 * PA9 = CS
 * PA10 = RS
 * PF0 = WR
 * PF1 = RD
 * 
 * Display is configured in 8 colour mode.
 * This simplifies decoding of reads from video memory and lowers power consumption
*/
void display::begin(timer &t)
{    
    // Low level I/O port initialisation
    // Turn on GPIO A,B and F
    RCC->AHBENR |= BIT17 + BIT18 + BIT22;
    // Make all bits connected to the display outputs to start with
    // Must avoid changing the mode for PA13 and PA14 (SWD and SWCLK)    
    
    GPIOA->MODER &= 0xffc00000;
    GPIOA->MODER |= 0x155555; // Make all bits from 0 to 10 outputs (PA8 is not brought out to a pin so no need to worry about it)    
    
    // Make PB1 an output
    GPIOB->MODER &= ~(BIT3);
    GPIOB->MODER |= (BIT2);
    
    // Make PF0, PF1 outputs
    GPIOF->MODER &= 0xfffffff0;
    GPIOF->MODER |= (BIT0 + BIT2);
    
    // get the display started
    // The set of steps carried out here are specific to the type of display being used
        RDHigh();
    CSHigh();
    WRHigh();
    RSTHigh();
    RSHigh();
    
    
    t.sleep(10);    
    RSTLow();
    t.sleep(10);
    RSTHigh();
    t.sleep(100);
    RDHigh();
    CSHigh();
    WRHigh();
    t.sleep(100);
    CSLow();
    // Power control startup sequence (from manufacturer)
    t.sleep(150);
    writeCommandData(0x01, 0x011C); // driver output control (not mirrored, 220 lines)
    writeCommandData(0x07, 0x000f); // Display control 8 colour mode 
    writeCommandData(0x11, 0x1000); // Power control
    writeCommandData(0x02, 0x0000); // LCD driving wave control 0 : Column Inversion
    writeCommandData(0xB0, 0x1412); // Power control(0c12)
    writeCommandData(0x0B, 0x0000); // Frame Rate Control 4-bit
    t.sleep(100);

    
    writeCommand((uint16_t) 0x22);    
    // black out the screen 
    fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00);                    

}
// Graphics functions
void display::putPixel(uint16_t x, uint16_t y, uint16_t colour)
{
    openAperture(x, y, x + 1, y + 1);
    writeData(colour); 
}
uint16_t display::getPixel(uint16_t x, uint16_t y)
{
    writeCommandData(0x20,x); // set x, y coordinate.
    writeCommandData(0x21,y);
    writeCommand((uint16_t) 0x22);    // put in to read/write mode
    return readData(); // get the data
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
            writeData(Colour);
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
            writeData(Colour);
        }
    }
}
void display::drawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t Colour)
{
    drawLine(x,y,x+w,y,Colour);
    drawLine(x,y,x,y+h,Colour);
    drawLine(x+w,y,x+w,y+h,Colour);
    drawLine(x,y+h,x+w,y+h,Colour);
}
void display::drawCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour)
{
// Reference : https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);
    if (radius > x0)
        return; // don't draw even parially off-screen circles
    if (radius > y0)
        return; // don't draw even parially off-screen circles
        
    if ((x0+radius) > SCREEN_WIDTH)
        return; // don't draw even parially off-screen circles
    if ((y0+radius) > SCREEN_HEIGHT)
        return; // don't draw even parially off-screen circles    
    while (x >= y)
    {
        putPixel(x0 + x, y0 + y, Colour);
        putPixel(x0 + y, y0 + x, Colour);
        putPixel(x0 - y, y0 + x, Colour);
        putPixel(x0 - x, y0 + y, Colour);
        putPixel(x0 - x, y0 - y, Colour);
        putPixel(x0 - y, y0 - x, Colour);
        putPixel(x0 + y, y0 - x, Colour);
        putPixel(x0 + x, y0 - y, Colour);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        
        if (err > 0)
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}
void display::fillCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour)
{
// Reference : https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
// Similar to drawCircle but fills the circle with lines instead
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    if (radius > x0)
        return; // don't draw even parially off-screen circles
    if (radius > y0)
        return; // don't draw even parially off-screen circles
        
    if ((x0+radius) > SCREEN_WIDTH)
        return; // don't draw even parially off-screen circles
    if ((y0+radius) > SCREEN_HEIGHT)
        return; // don't draw even parially off-screen circles        
    while (x >= y)
    {
        drawLine(x0 - x, y0 + y,x0 + x, y0 + y, Colour);        
        drawLine(x0 - y, y0 + x,x0 + y, y0 + x, Colour);        
        drawLine(x0 - x, y0 - y,x0 + x, y0 - y, Colour);        
        drawLine(x0 - y, y0 - x,x0 + y, y0 - x, Colour);        

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        
        if (err > 0)
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}
void display::drawLineLowSlope(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1, uint16_t Colour)
{
    // Reference : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm    
  int dx = x1 - x0;
  int dy = y1 - y0;
  int yi = 1;
  if (dy < 0)
  {
    yi = -1;
    dy = -dy;
  }
  int D = 2*dy - dx;
  
  int y = y0;

  for (int x=x0; x <= x1;x++)
  {
    putPixel(x,y,Colour);    
    if (D > 0)
    {
       y = y + yi;
       D = D - 2*dx;
    }
    D = D + 2*dy;
    
  }
}

void display::drawLineHighSlope(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1, uint16_t Colour)
{
        // Reference : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

  int dx = x1 - x0;
  int dy = y1 - y0;
  int xi = 1;
  if (dx < 0)
  {
    xi = -1;
    dx = -dx;
  }  
  int D = 2*dx - dy;
  int x = x0;

  for (int y=y0; y <= y1; y++)
  {
    putPixel(x,y,Colour);
    if (D > 0)
    {
       x = x + xi;
       D = D - 2*dy;
    }
    D = D + 2*dx;
  }
}
void display::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t Colour)
{
    // Reference : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    if ( iabs(y1 - y0) < iabs(x1 - x0) )
    {
        if (x0 > x1)
        {
            drawLineLowSlope(x1, y1, x0, y0, Colour);
        }
        else
        {
            drawLineLowSlope(x0, y0, x1, y1, Colour);
        }
    }
    else
    {
        if (y0 > y1) 
        {
            drawLineHighSlope(x1, y1, x0, y0, Colour);
        }
        else
        {
            drawLineHighSlope(x0, y0, x1, y1, Colour);
        }
        
    }
}

void display::print(const char *Text, uint16_t len, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour)
{
        // This function draws each character individually.  It uses an array called TextBox as a temporary storage
    // location to hold the dots for the character in question.  It constructs the image of the character and then
    // calls on putImage to place it on the screen
    uint8_t Index = 0;
    uint8_t Row, Col;
    const uint8_t *CharacterCode = 0;    
    uint16_t TextBox[FONT_WIDTH * FONT_HEIGHT];
    for (Index = 0; Index < len; Index++)
    {
        CharacterCode = &Font5x7[FONT_WIDTH * (Text[Index] - 32)];
        Col = 0;
        while (Col < FONT_WIDTH)
        {
            Row = 0;
            while (Row < FONT_HEIGHT)
            {
                if (CharacterCode[Col] & (1 << Row))
                {
                    TextBox[(Row * FONT_WIDTH) + Col] = ForeColour;
                }
                else
                {
                    TextBox[(Row * FONT_WIDTH) + Col] = BackColour;
                }
                Row++;
            }
            Col++;
        }
        putImage(x, y, FONT_WIDTH, FONT_HEIGHT, (const uint16_t *)TextBox);
        x = x + FONT_WIDTH + 2;
    }
}
void display::print(uint16_t Number, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour)
{
     // This function converts the supplied number into a character string and then calls on puText to
    // write it to the display
    char Buffer[5]; // Maximum value = 65535
    Buffer[4] = Number % 10 + '0';
    Number = Number / 10;
    Buffer[3] = Number % 10 + '0';
    Number = Number / 10;
    Buffer[2] = Number % 10 + '0';
    Number = Number / 10;
    Buffer[1] = Number % 10 + '0';
    Number = Number / 10;
    Buffer[0] = Number % 10 + '0';
    print(Buffer, 5, x, y, ForeColour, BackColour);
}
char HexChar(char value)
{
    if (value < 10)
        return value + '0';
    else
        return value + 55;
}
void display::printHex(uint16_t Number, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour)
{
     // This function converts the supplied number into a character string and then calls on puText to
    // write it to the display
    char Buffer[4]; // Maximum value = ffff
    Buffer[3] = HexChar(Number % 16);
    Number = Number >> 4;
    Buffer[2] = HexChar(Number % 16);
    Number = Number >> 4;
    Buffer[1] = HexChar(Number % 16);
    Number = Number >> 4;
    Buffer[0] = HexChar(Number % 16);
    print(Buffer, 4, x, y, ForeColour, BackColour);
}
void display::resetDisplay()
{
    volatile int i=0; // don't optimize this variable out
    RSTLow();
    while(i < 20000) i++; // delay approx 12ms 
    RSTHigh();
    i=0;
    while(i < 20000) i++;
}
void display::writeCommand(uint8_t Cmd)
{
    RSLow();    
    output_byte(Cmd);
}
void display::writeCommand(uint16_t Cmd)
{    
    RSLow();    
    output_byte(Cmd >> 8);
    WRLow();
    WRHigh();
    output_byte(Cmd);
    WRLow();
    WRHigh();
}
void display::writeData(uint8_t Data)
{
    RSHigh();
    output_byte(Data);
}
void display::writeData(uint16_t Data)
{
    RSHigh();
    output_byte(Data >> 8);
    WRLow();
    WRHigh();
    output_byte(Data);
    WRLow();
    WRHigh();
}
void delay(volatile int dly)
{
    while(dly--);
}
uint16_t display::readData()
{
    uint16_t returnValue = 0;
    output_byte(0);
    uint32_t SavedModeA = GPIOA->MODER;
    
    GPIOA->MODER &= ~(0xffff); // make lower 8 bits input bits    
    RSHigh();     
    RDHigh();        
    RDLow(); // dummy read    
    RDHigh();    
    RDLow(); // dummy read    
    RDHigh();
    
    RDLow(); // first read   
    RDHigh();    
    returnValue = (GPIOA->IDR & 0xff) << 8;    
    
    RDLow(); // second read    
    RDHigh();    
    returnValue += (GPIOA->IDR & 0xff);    
    GPIOA->MODER = SavedModeA;
   
    return returnValue;
}

void display::writeCommandData(uint16_t Cmd, uint16_t Data)
{
    writeCommand(Cmd);
    writeData(Data);
}
void display::openAperture(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    // open up an area for drawing on the display
    
    writeCommandData(0x36,x2); 
    writeCommandData(0x37,x1);
    writeCommandData(0x38,y2);
    writeCommandData(0x39,y1); 
    writeCommandData(0x20,x1);
    writeCommandData(0x21,y1); 	
    writeCommand((uint16_t)0x22);
}
inline void display::WRLow()
{
    GPIOF->BRR = BIT0; 
}
inline void display::WRHigh()
{
    GPIOF->BSRR = BIT0;
}
inline void display::CSLow()
{
    GPIOA->BRR = BIT9;
}
inline void display::CSHigh()
{
    GPIOA->BSRR = BIT9;
}
inline void display::RDLow()
{
    GPIOF->BRR = BIT1;
}
inline void display::RDHigh()
{
    GPIOF->BSRR = BIT1;
}
inline void display::RSTLow()
{
    GPIOB->BRR = BIT1;
}
inline void display::RSTHigh()
{
    GPIOB->BSRR = BIT1;
}
inline void display::RSLow()
{
    GPIOA->BRR = BIT10;
}
inline void display::RSHigh()
{
    GPIOA->BSRR = BIT10;        
}
inline void display::output_byte(uint8_t b)
{
    GPIOA->BRR = 0xff; // clear all 8 bits first
    GPIOA->BSRR = b;   // set relevant bits    
}

/* 
 * The touch screen interface:
 * This is a resistive touch screen.  The way it works is as follows:
 * To read the X value, Drive one end of the X axis high, the other low (digital outputs)
 * You then make one end of the Y axis a digital input (high impendance)
 * The other end of the Y axis you make an analogue input.
 * The analogue value you read depends on where the user pushes on the screen
 * Wiring : 
 * PA6 = ADC_IN6 = X+
 * PA7 = ADC_IN7 = Y-
 * PA10 (RS) = X- 
 * PF0 (WR) = Y+
 *  
*/
uint16_t display::readXTouch()
{
    int16_t result=0;
    CSHigh(); // make the display logic ignore what happens next
    uint32_t SavedModeA,SavedModeF;
    SavedModeA = GPIOA->MODER;
    SavedModeF = GPIOF->MODER;
    // Want to configure PA7 as an analogue input, PF0 as a digital input
    GPIOA->MODER |= (BIT15+BIT14); 
    GPIOF->MODER &= ~(BIT0 + BIT1); 
    GPIOA->BSRR |= BIT6; // Drive X+ high
    GPIOA->BRR |= BIT10; // Drive X- Low
    // Select ADC channel 7
    ADC->CHSELR = BIT7;
    ADC->CR |= BIT0;  // enable ADC
    // Start ADC 
    ADC->CR |= BIT2; 
    // Wait for end-of-conversion
    while((ADC->ISR & BIT2)==0);
    // read data
    result = ADC->DR;
    
    GPIOA->MODER = SavedModeA;
    GPIOF->MODER = SavedModeF;        
    CSLow(); // make the display logic pay attention to the bus
    if ( (result >= TOUCH_X_MIN) && (result <=TOUCH_X_MAX) )
    {
        result = SCREEN_WIDTH- ( (result - TOUCH_X_MIN)*SCREEN_WIDTH / (TOUCH_X_MAX - TOUCH_X_MIN));
    }
    else
    {
        result = -1;
    }
     
    
    return result;
}

uint16_t display::readYTouch()
{
    int16_t result=0;
    CSHigh(); // make the display logic ignore what happens next
    uint32_t SavedModeA,SavedModeF;
    SavedModeA = GPIOA->MODER;
    SavedModeF = GPIOF->MODER;
    
    // Want to configure PA6 as an analogue input, PA10 as a digital input
    GPIOA->MODER |= (BIT13+BIT12); 
    GPIOA->MODER &= ~(BIT20 + BIT21); 
    GPIOF->BSRR |= BIT0; // Drive Y+ high
    GPIOA->BRR |= BIT7; // Drive Y- Low    
    // Select ADC channel 6
    ADC->CHSELR = BIT6;    
    ADC->CR |= BIT0;  // enable ADC    
    // Start ADC 
    ADC->CR |= BIT2; 
    // Wait for end-of-conversion
    while(ADC->CR & BIT2);
    // read data
    result = ADC->DR;
    
    GPIOA->MODER = SavedModeA;
    GPIOF->MODER = SavedModeF;
    CSLow(); // make the display logic pay attention to the bus
    
    if ( (result >= TOUCH_Y_MIN) && (result <=TOUCH_Y_MAX) )
    {
        result = SCREEN_HEIGHT - ( (result - TOUCH_Y_MIN)*SCREEN_HEIGHT / (TOUCH_Y_MAX - TOUCH_Y_MIN));
    }
    else
    {
        result = -1;
    }
        
    return result;
}



uint16_t display::RGBToWord(uint16_t  RGB)
{
    
    /* In 8 colour mode, only three bits are necessary to set a colour
     * This means that the other 13 bis in a 16 bit video memory location are unused     
     * During a write, 
     * Bit 4 turns on red 
     * Bit 10 turns on green
     * Bit 15 turns on Blue
     * During a read things come out differently
     * It seems that bits 15,14,7 and 6 are always 1 which results in a hex value of 0xC0C0
     * for black.  
     * Bit 4 indicates Red is on
     * Bit 10 green
     * But bit 15 is always a 1 which means we don't know whether blue is on or off
     * The unused bits come in handy here.
     * An unused bit can be set to 1 along with bit 15 when turning on the blue colour to indicate that
     * blue is on or off.  In fact, we can simplify the decoding further by writing the 3 bit code (r.g.b)
     * to bits 11,10,9 (RGB)
     * Decoding a memory read is then easier - just shift the 16 bit value 9 bits to the right and 
     * mask off all bits except the 3 LSB's      
     * 
     * There is a further benefit - additional data can be stored about each pixel in the remaining
     * bits.  
     * To recap : Bit layout during a read/write as it stands
     * 
     * B100 RGB0 110R 0000
     * So bits 13,12,8,5,3,2,1 and 0 are unused. Functions encodePixelMetaData and decodePixelMetaData 
     * are now possible - all within display RAM (not MCU RAM) - very useful for games.
     */
    uint16_t Colour=0;
    // Red
    if (RGB & BIT2) 
        Colour |= 0x10;   // set red bit
    
    // Green + additional Red and Blue bits for fast decoding
    Colour = Colour | ((RGB & 0x7) << 9);                          
    // Blue
    if (RGB & BIT0)
        Colour |= 0x8000; // set blue bit                          
    return Colour;
}
uint16_t display::WordToRGB(uint16_t  w)
{
    uint16_t rgb=0;
    return ((w >> 9) & 7);

}
uint16_t display::encodePixelMetaData(uint16_t PixelData, uint16_t MetaData)
{
    PixelData = PixelData | ( (MetaData & (BIT7 + BIT6)) << 6); // Bits 13 and 12
    PixelData = PixelData | ( (MetaData & (BIT5) ) << 3); // Bit 8
    PixelData = PixelData | ( (MetaData & (BIT4) ) << 1); // Bits 5
    PixelData = PixelData | (MetaData & ( BIT3+BIT2+BIT1+BIT0)); // Bits 3-0
    return PixelData;
}
uint16_t display::decodePixelMetaData(uint16_t data)
{
    uint16_t MetaData = 0;
    MetaData |= ( ( data & (BIT13+BIT12) ) >> 6);
    MetaData |= ( ( data & (BIT8) ) >> 3);
    MetaData |= ( ( data & (BIT5) ) >> 1);
    MetaData |= data & (BIT3+BIT2+BIT1+BIT0);            
    return MetaData;
}
 

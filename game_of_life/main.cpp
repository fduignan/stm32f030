// Initial test program for ST7789
#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "display.h"
#include "timer.h"
#define PSTRING(str) str,sizeof(str)-1
timer Timer;
display Display;
uint32_t shift_register;
void initADC()
{
    RCC->APB2ENR |= (1 << 9); // enabled ADC clock  
    ADC->CR = 0;    
    ADC->CCR = (1 << 23) + (1 << 22); // enable reference and temp sensor    
    ADC->CR |= (1 << 31); // start calibration
    while ( ADC->CR & (1 << 31) ); // wait for calibration to complete
    ADC->SMPR = 0x06; // S+H Time = 71.5 ADC cycles
    ADC->CR = 1; // enable the ADC
}

uint32_t getAnalogueNoise() // used to see the random number generator
{       
    uint16_t result = 0;    
    // Select ADC channel 8 - not brought out to pin.
    GPIOA->MODER |=(BIT17 + BIT16);
    ADC->CHSELR = BIT8;    
    // Start ADC 
    ADC->CR |= BIT0;
    ADC->CR |= BIT2; 
    // Wait for end-of-conversion
    while((ADC->ISR & BIT2)==0);
    // read data
    result = ADC->DR;            
    return result;
}
uint32_t prbs()
{
	// This is an unverified 31 bit PRBS generator
	// It should be maximum length but this has not been verified 
	unsigned long new_bit=0;
	static int busy=0; // need to prevent re-entrancy here
	if (!busy)
	{
		busy=1;
		new_bit= ((shift_register & (1<<27))>>27) ^ ((shift_register & (1<<30))>>30);
		new_bit= ~new_bit;
		new_bit = new_bit & 1;
		shift_register=shift_register << 1;
		shift_register=shift_register | (new_bit);
		busy=0;
	}
	return shift_register & 0x7ffffff; // return 31 LSB's 
}
void random_seed()
{
    // Need to find a source of random data.  
    // Will add up and scale a few reads of noise to change more bits in the shift register
    shift_register = 0;
    while(shift_register == 0)
    {
        for (int i=1;i<17;i++)
            shift_register += getAnalogueNoise()<<i;
    }   
}
uint32_t random(uint32_t lower,uint32_t upper)
{
    return (prbs()%((upper+1)-lower))+lower;
}
void setup()
{    
    Timer.begin();
    Display.begin(Timer);
    initADC();
    random_seed();   
}
void testRoutine()
{
    uint8_t Count=0;
    uint16_t x,y;
    uint16_t Colour=0;
    uint16_t Pixel =0;
    while (1) 
    {
        // Test routines: 
        // Show X,Y touch
        x = Display.readXTouch();
        
        y = Display.readYTouch();
        if ((x < SCREEN_WIDTH) && (y < SCREEN_HEIGHT))
        {
            Display.print(y,1,70,Display.RGBToWord(0b110),0);            
            Display.print(x,1,40,Display.RGBToWord(0b001),0);            
        }
        Count++;
        Display.print(Count,1,110,Display.RGBToWord(0b100),0);            
                
        for (Colour=0;Colour<8;Colour++)
        {
            Display.fillRectangle(0,0,176,220,Display.encodePixelMetaData(Display.RGBToWord(Colour),Count));
            //Display.putPixel(1,1,Display.encodePixelMetaData(Display.RGBToWord(Colour),Count));
            Pixel = Display.getPixel(1,1);
            if (Display.decodePixelMetaData(Pixel) != Count)
            {
                Display.print( "Error" ,sizeof("Error")-1,1,130,Display.RGBToWord(0b111),0);                
                Display.printHex(Count,1,150,Display.RGBToWord(0b111),0);               
            }
            if (Display.WordToRGB(Pixel) != Colour)
            {
                Display.print( "Error" ,sizeof("Error")-1,1,150,Display.RGBToWord(0b101),0);                
                Display.printHex(Colour,1,160,Display.RGBToWord(0b101),0);               
            }
            Count++;
            Timer.sleep(20);
        }
        
    }
}
uint16_t wrap(uint16_t value, uint16_t extents)
{
    // Wraps the given value with the bounds of 0 to extents-1    
    uint16_t returnValue = extents + value;
    while(returnValue >= extents)
    {
        returnValue -= extents;
    }
    return returnValue;
    
}
uint16_t getChildColour(uint16_t SurroundingCellColours[])
{
    // Assuming that there are only 3 live surrounding cells.
    int red,blue,yellow,green;
    red=blue=green=yellow= 0;    
    for (int i=0;i < 8;i++)
    {
        if (SurroundingCellColours[i] == 0b001)
            blue++;
        if (SurroundingCellColours[i] == 0b010)
            green++;
        if (SurroundingCellColours[i] == 0b100)
            red++;
        if (SurroundingCellColours[i] == 0b110)
            yellow++;                
    }
/*
If two parents have the same color (i.e. they form a majority), the child has the same color. 
If there is no majority (i.e. all 3 parents have different colors), the child is of the fourth color.
*/
    if (red > 1) return 0b100;
    if (blue > 1) return 0b001;
    if (green > 1) return 0b010;
    if (yellow > 1) return 0b110;
// ok, no majority, so return the "other" colour
    if (red == 0) return 0b100;
    if (blue == 0) return 0b001;
    if (green == 0) return 0b010;
    // last option,  (done this way to stop compiler warning)
    return 0b110;
    
}
int main()
{        
    
    setup();
   // testRoutine();
    
    int Next=0;
    uint16_t x,y;
    uint16_t Pixel = 0;
    
    Display.fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Display.RGBToWord(0));
    Display.print(PSTRING("The game of life"),25,10,Display.RGBToWord(0b111),0);
    Display.print(PSTRING("Draw your yellow critters"),1,30,Display.RGBToWord(0b111),0);
    Display.print(PSTRING("then press Blue"),1,40,Display.RGBToWord(0b111),0);
    Timer.sleep(2000);
    Display.fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Display.RGBToWord(0b000));
    Display.fillRectangle(SCREEN_WIDTH/2 - 20 , SCREEN_HEIGHT-40, 40, 15, Display.RGBToWord(0b111));
    Display.print(PSTRING("Blue"),SCREEN_WIDTH/2 - 17,SCREEN_HEIGHT-36,Display.RGBToWord(0b001),Display.RGBToWord(0b111));
// The user will now draw the initial cells on the screen.  
// There will be 4 colours in total.          
    while(!Next)
    {
        x = Display.readXTouch();        
        y = Display.readYTouch();
        if ((x < SCREEN_WIDTH) && (y < SCREEN_HEIGHT))
        { // valid touch
            if ( (x >= SCREEN_WIDTH/2 - 20) && (x <= SCREEN_WIDTH/2 + 20) )
            {
                if ((y>=SCREEN_HEIGHT-40) && (y <= SCREEN_HEIGHT-25))
                {
                    Next = 1;
                }
            }                        
            if (!Next)
            {
                Display.putPixel(x,y,Display.RGBToWord(0b110));                
            }
            
        }
        Timer.sleep(5);
    }
    
    Timer.sleep(200);
    Next = 0;
    Display.fillRectangle(SCREEN_WIDTH/2 - 20 , SCREEN_HEIGHT-40, 40, 15, Display.RGBToWord(0b111));
    Display.print(PSTRING("Red"),SCREEN_WIDTH/2 - 17,SCREEN_HEIGHT-36,Display.RGBToWord(0b001),Display.RGBToWord(0b111));
    while(!Next)
    {
        x = Display.readXTouch();        
        y = Display.readYTouch();
        if ((x < SCREEN_WIDTH) && (y < SCREEN_HEIGHT))
        { // valid touch
            if ( (x >= SCREEN_WIDTH/2 - 20) && (x <= SCREEN_WIDTH/2 + 20) )
            {
                if ((y>=SCREEN_HEIGHT-40) && (y <= SCREEN_HEIGHT-25))
                {
                    Next = 1;
                }
            }                        
            if (!Next)
            {
                Display.putPixel(x,y,Display.RGBToWord(0b001));                
            }
            
        }
        Timer.sleep(5);
    }
    Timer.sleep(200);
    Next = 0;
    Display.fillRectangle(SCREEN_WIDTH/2 - 20 , SCREEN_HEIGHT-40, 40, 15, Display.RGBToWord(0b111));
    Display.print(PSTRING("Green"),SCREEN_WIDTH/2 - 17,SCREEN_HEIGHT-36,Display.RGBToWord(0b001),Display.RGBToWord(0b111));
    while(!Next)
    {
        x = Display.readXTouch();        
        y = Display.readYTouch();
        if ((x < SCREEN_WIDTH) && (y < SCREEN_HEIGHT))
        { // valid touch
            if ( (x >= SCREEN_WIDTH/2 - 20) && (x <= SCREEN_WIDTH/2 + 20) )
            {
                if ((y>=SCREEN_HEIGHT-40) && (y <= SCREEN_HEIGHT-25))
                {
                    Next = 1;
                }
            }                        
            if (!Next)
            {
                Display.putPixel(x,y,Display.RGBToWord(0b100));                
            }
            
        }
        Timer.sleep(5);
    }

    Timer.sleep(200);
    Next = 0;
    Display.fillRectangle(SCREEN_WIDTH/2 - 20 , SCREEN_HEIGHT-40, 40, 15, Display.RGBToWord(0b111));
    Display.print(PSTRING("Done"),SCREEN_WIDTH/2 - 17,SCREEN_HEIGHT-36,Display.RGBToWord(0b001),Display.RGBToWord(0b111));
    while(!Next)
    {
        x = Display.readXTouch();        
        y = Display.readYTouch();
        if ((x < SCREEN_WIDTH) && (y < SCREEN_HEIGHT))
        { // valid touch
            if ( (x >= SCREEN_WIDTH/2 - 20) && (x <= SCREEN_WIDTH/2 + 20) )
            {
                if ((y>=SCREEN_HEIGHT-40) && (y <= SCREEN_HEIGHT-25))
                {
                    Next = 1;
                }
            }                        
            if (!Next)
            {
                Display.putPixel(x,y,Display.RGBToWord(0b010));                
            }
            
        }
        Timer.sleep(5);
    }
    
        
    Display.fillRectangle(SCREEN_WIDTH/2 - 20 , SCREEN_HEIGHT-40, 40, 15, Display.RGBToWord(0b0));
    // All colours drawn, now play the game
/*
From wikipedia: The basic rules of monochrome Game of Life
Any alive cell that is touching less than two alive neighbours dies.
Any alive cell touching four or more alive neighbours dies.
Any alive cell touching two or three alive neighbours does nothing.
Any dead cell touching exactly three alive neighbours becomes alive.
 
To add colours : From conwaylife.com 
 
"Quad-Life is a coloring variant that uses four different symmetrical colors. The two rules that determines the color of births are:

If two parents have the same color (i.e. they form a majority), the child has the same color. (This is the same as Life and Immigration).
If there is no majority (i.e. all 3 parents have different colors), the child is of the fourth color.
Quad-Life is a superset of both normal Life and Immigration. Its most interesting feature is that populations where one color has totally died out can give birth to that same color again. Only if two colors die out (reducing this to Immigration) is such color death necessarily permanent.

Also, in many cases, oscillator and spaceships that include 3 or 4 colors may have higher periods than their normal Life counterparts, cycling through several different color combinations. For example, the 4-color Glider has one side of one color, while the two bits on the other side are of different colors, and cycle through the three different permutations of the two other colors, giving the resulting multi-color Glider a period of 12, rather than the usual 4. Whenever such multi-colored objects have one or more periods that differ from those of the original objects, those periods are shown in parentheses at the end (if known). "

A further complication is the need to "wrap the landscape around" i.e. the cell above y=0 is at y=SCREEN_HEIGHT-1
The cell below SCREEN_HEIGHT - 1 is at 0 i.e. height is modulo SCREEN_HEIGHT
Similarly for x position : All positions are modulo SCREEN_WIDTH

This game will render in two passes.  On the first pass, the next state of each cell (pixel) is calculated and stored in the pixel metadata
During pass 2 all pixel colours are set to their new pixel values
*/


    // Test shape to check that all is well
    /*Display.putPixel(10,200,Display.RGBToWord(0b110));
    Display.putPixel(10,201,Display.RGBToWord(0b110));
    Display.putPixel(10,202,Display.RGBToWord(0b110));*/
    while(1)
    {
        // Pass 1
        for (y = 0; y < SCREEN_HEIGHT; y++)
        {
            for (x = 0; x < SCREEN_WIDTH; x++)
            {
                // Starting with the pixel at x,y find out about all of the pixels surrounding it.
                // There are a total of 8 cells surrouning any one cell
                // First go of this will not be optimized in any way - it may be fast enough anyway
                uint16_t SurroundingCells[8];
                uint16_t thisCell = Display.WordToRGB(Display.getPixel(x,y)); 
                SurroundingCells[0] = Display.WordToRGB(Display.getPixel(wrap(x-1,SCREEN_WIDTH),wrap(y-1,SCREEN_HEIGHT)));
                SurroundingCells[1] = Display.WordToRGB(Display.getPixel(wrap(x  ,SCREEN_WIDTH),wrap(y-1,SCREEN_HEIGHT))); 
                SurroundingCells[2] = Display.WordToRGB(Display.getPixel(wrap(x+1,SCREEN_WIDTH),wrap(y-1,SCREEN_HEIGHT))); 
                SurroundingCells[3] = Display.WordToRGB(Display.getPixel(wrap(x-1,SCREEN_WIDTH),wrap(y  ,SCREEN_HEIGHT))); 
                SurroundingCells[4] = Display.WordToRGB(Display.getPixel(wrap(x+1,SCREEN_WIDTH),wrap(y  ,SCREEN_HEIGHT))); 
                SurroundingCells[5] = Display.WordToRGB(Display.getPixel(wrap(x-1,SCREEN_WIDTH),wrap(y+1,SCREEN_HEIGHT))); 
                SurroundingCells[6] = Display.WordToRGB(Display.getPixel(wrap(x  ,SCREEN_WIDTH),wrap(y+1,SCREEN_HEIGHT))); 
                SurroundingCells[7] = Display.WordToRGB(Display.getPixel(wrap(x+1,SCREEN_WIDTH),wrap(y+1,SCREEN_HEIGHT))); 
                int SurroundingCellsAlive = 0;
                for (int cellindex=0; cellindex < 8; cellindex++)
                {
                    if (SurroundingCells[cellindex]> 0)  // Count up the surrounding cells that are alive
                        SurroundingCellsAlive++;
                }
                if (thisCell == 0) // Is this cell dead?
                {
                    if ((SurroundingCellsAlive == 3))
                    {
                        // Cell will come alive.
                        // Now what colour should it be? - depends on the parents                        
                        thisCell = Display.encodePixelMetaData(0,getChildColour(SurroundingCells));
                                                                        
                    }
                }
                else
                {
                    // Is this cell alive and due to die?
                    if (SurroundingCellsAlive > 3)
                    {
                        // Death from starvation
                        thisCell = Display.encodePixelMetaData(Display.RGBToWord(thisCell),0);
                    }
                    else if (SurroundingCellsAlive < 2)
                    {
                        // Death from loneliness
                        thisCell = Display.encodePixelMetaData(Display.RGBToWord(thisCell),0);
                        
                    }
                    else
                    {
                        thisCell = Display.encodePixelMetaData(Display.RGBToWord(thisCell),thisCell);
                    }
                }
                Display.putPixel(x,y,thisCell); // Write back the pixel with it's metadata
            } 
        }
        // Pass 2
        for (y = 0; y < SCREEN_HEIGHT; y++)
        {
            for (x = 0; x < SCREEN_WIDTH; x++)
            {
                Pixel = Display.getPixel(x,y);
                
                Display.putPixel(x,y,Display.RGBToWord(Display.decodePixelMetaData(Pixel)));
            }
        }            
    }
    
}

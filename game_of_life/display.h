#include <stdint.h>
#include "timer.h"
// Assumption: Top left of display is 0,0
#define SCREEN_WIDTH 176
#define SCREEN_HEIGHT 220
// The following values were found by experiment
#define TOUCH_X_MAX 3700
#define TOUCH_X_MIN 320
#define TOUCH_Y_MAX 3700
#define TOUCH_Y_MIN 600
// Define a macro to allow easy definition of colours
// Format of colour value: <BGND 1 bit><Red 5 bits><Green 5 bits><Blue 5 bits>
//#define RGBToWord( R,  G,  B)  (  ((R&0xf8) << (11-3)) | ((G&0x72) << (4)) | ((B&0xf0)>>3) )

class display
{
public:
    display(){};
    void begin(timer &t);
    void putPixel(uint16_t x, uint16_t y, uint16_t colour);
    uint16_t getPixel(uint16_t x, uint16_t y);
    void putImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *Image);
    void fillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t Colour);
    void drawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t Colour);
    void drawCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour);
    void fillCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour);
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t Colour);
    void print(const char *Text, uint16_t len, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour);
    void print(uint16_t Number, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour);
    void printHex(uint16_t Number, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour);
    uint16_t readXTouch();
    uint16_t readYTouch();
    uint16_t RGBToWord(uint16_t  RGB);
    uint16_t WordToRGB(uint16_t  RGB);
    uint16_t encodePixelMetaData(uint16_t PixelData, uint16_t MetaData);
    uint16_t decodePixelMetaData(uint16_t data);

    
private:
    void drawLineLowSlope(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1, uint16_t Colour);
    void drawLineHighSlope(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1, uint16_t Colour);
    void RSLow();
    void RSHigh();
    void resetDisplay();
    void writeCommand(uint8_t Cmd);
    void writeData(uint8_t Data);    
    void writeCommand(uint16_t Cmd);
    void writeData(uint16_t Data);
    uint16_t readData(  );
    void writeCommandData(uint16_t Cmd,uint16_t Data);
    void openAperture(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);            
    void WRLow();
    void WRHigh();
    void CSLow();
    void CSHigh();
    void RDLow();
    void RDHigh();
    void RSTLow();
    void RSTHigh();       
    void output_byte(uint8_t b);
    int iabs(int x) // simple integer version of abs for use by graphics functions
    {
        if (x < 0)
            return -x;
        else
            return x;
    }

    
};

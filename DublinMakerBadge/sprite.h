    #include <stdint.h>
#include "console.h"
extern console Console;
class sprite {
public:
    sprite( const uint16_t * image, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void show();
    void redraw();
    void hide();
    void move(uint16_t x, uint16_t y);
    uint16_t within(uint16_t x, uint16_t y);
    uint16_t touching(uint16_t x, uint16_t y);
    inline uint16_t getX() {
        return X;
    }
    inline uint16_t getY() {
        return Y;
    }
    inline uint8_t getWidth() {
        return w;
    }
    inline uint8_t getHeight() {
        return h;
    }
    
    
    inline uint16_t visible() {    
        return Visible;
    }
    
private:   
    uint16_t w,h;
    const uint16_t *img;
    uint16_t X,Y;
    uint8_t Visible;    
};

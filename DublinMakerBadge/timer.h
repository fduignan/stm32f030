#ifndef __timer_h
#define __timer_h
#include <stdint.h>
#define MAX_TIMER_CALLBACKS 8
typedef void (*fptr)(void);    
class timer {
public:
    timer(){};
    void begin();
    int attach(fptr cb);
    int detach(fptr cb);
    void sleep(uint32_t dly);
private:
    uint32_t milliseconds;
    friend void Systick_Handler(void);
   
    fptr CallbackArray[MAX_TIMER_CALLBACKS];
};
#endif

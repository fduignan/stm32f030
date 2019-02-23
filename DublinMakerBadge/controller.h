#include <stdint.h>

class controller
{
public:
    controller(){};
    void begin();
    uint16_t getButtonState();    
    uint16_t readTemperature();
    void stopADC();
private:

};

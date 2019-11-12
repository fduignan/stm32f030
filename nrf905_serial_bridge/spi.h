#ifndef __spi_h
#define __spi_h
class spi {
public:
    spi() { };
    int begin();        
    uint8_t transferSPI(uint8_t data);
    uint8_t transferSPI(uint16_t data);

};
#endif

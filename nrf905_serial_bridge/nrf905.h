#ifndef __nrf905_h
#define __nrf905_h
// Interface library for a connection between an STM32F030 and an NRF905 radio module
// The constructor is deliberately blank as the constructor call order can be a little confusing
// The device is initialized with the 'begin' method
#include "timer.h"
#include "spi.h"

#define PAYLOAD_LENGTH 32
struct NRF905DataPacket {
    uint8_t Address[4];   
    uint8_t Payload[PAYLOAD_LENGTH];
};
class nrf905 {
public:
    nrf905() { };
    int begin(timer *pTimer,spi * SPI, uint8_t *rx_address, uint8_t *tx_address, uint32_t channel);
    int DataReady();    
    uint8_t getStatus();
    void writeTXPayload(uint8_t *Payload, unsigned length);
    void readTXPayload(uint8_t *Payload, unsigned length);
    void readRXPayload(uint8_t *Payload, unsigned length);        
    void readRXAddress(uint8_t *addr);
    void setRXAddress(const uint8_t *addr);
    void readTXAddress(uint8_t *addr);
    void setTXAddress(const uint8_t *addr);
    void setChannel(int Channel);
    void setBand(int Band);
    void setAutoRetran(int Auto);
    void setTXPower(int pwr);
    void setRXLowPower(int pwr);
    void setRXAFW(int afw);
    void setTXAFW(int afw);
    void writeRegister(int RegNum, uint8_t contents);
    uint8_t readRegister(int RegNum);     
    void transmit(NRF905DataPacket *Pkt);
    void RXMode();
    void LPMode();
private:    
    spi *pSPI;
    timer *pTimer;
    uint8_t Status;    
    void TXEnHigh();
    void TXEnLow();    
    void CSNLow();
    void CSNHigh();
    void CELow();
    void CEHigh();
    void PwrHigh();
    void PwrLow();    

    

};
#endif

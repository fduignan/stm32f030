// RX VERSION
#include <stdint.h>
#include "timer.h"
#include "uart.h"
#include "spi.h"
#include "nrf905.h"
#include "../include/STM32F0x0.h"
uint32_t i=0;

timer Timer;    
uart serial;
spi SPI;
nrf905 NRF905;
uint8_t TXAddr[8];
uint8_t RXAddr[8];

void delay(volatile uint32_t dly)
{
    while(dly--);
}
void setup()
{
    Timer.begin();
    serial.begin();    
    SPI.begin();    
    NRF905.begin(&Timer,&SPI,TXAddr,RXAddr,2);
    enable_interrupts();
}
void ShowRegisters()
{
    uint8_t buffer[4];
    serial.print("*** RF Config\r\n");
    for (int i=0;i<10;i++)
    {
        uint8_t value = NRF905.readRegister(i);        
        serial.print(value);
        serial.print(" ");
    }
    
    serial.print("\r\nRX Address");
    NRF905.readRXAddress((uint8_t *)buffer);
    for (int i=0;i < 4; i++)
    {
        serial.print(buffer[i]);
    }
    serial.print("\r\nTX Address");
    NRF905.readTXAddress((uint8_t *)buffer);
    for (int i=0;i < 4; i++)
    {
        serial.print(buffer[i]);
    }
    serial.print("\r\nSt: ");
    serial.print(NRF905.getStatus());    
    serial.print("\r\nDR: ");
    serial.print((uint8_t)NRF905.DataReady());
    serial.print("\r\n");
}
int main()
{
    uint8_t addr[] = {0xAA,0xAA,0xAA,0x01};
    uint32_t msg=0;
    uint16_t ch=0;
    setup();            
    NRF905.setRXAddress(addr);
    addr[3] = 2;
    NRF905.setRXAFW(4);
    NRF905.setTXAddress(addr);
    NRF905.setChannel(106); // 106 -> 433MHz
    NRF905.setBand(0);
    NRF905.setTXPower(3);    
    NRF905.PwrHigh();
    NRF905.CEHigh();
    serial.print("RX Mode\r\n");
    while(1)
    {        
        
        ShowRegisters();                
        
        if (NRF905.DataReady())
        {
            uint8_t message[32];
            NRF905.readRXPayload(message,sizeof(message));
            serial.print("\r\nRX\r\n");
            for (int i=0;i < sizeof(message); i++)
            {
                serial.print(message[i]);
                serial.print(" ");
            }
            serial.print("\r\n");
        }
        
        Timer.sleep(400);        
        
    }
}

// TX Version.  Sends a 32 byte packet twice a second at maximum power
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

uint8_t TXAddr[4]={0xAA,0xAA,0xAA,0x01};
uint8_t RXAddr[4]={0xAA,0xAA,0xAA,0x02};


void delay(volatile uint32_t dly)
{
    while(dly--);
}
void setup()
{
    Timer.begin();
    serial.begin();    
    SPI.begin();    
    NRF905.begin(&Timer,&SPI,RXAddr,TXAddr,106); // 106 -> 433MHz
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
    NRF905DataPacket pkt;
    pkt.Address[0]=0xaa;
    pkt.Address[1]=0xaa;
    pkt.Address[2]=0xaa;
    pkt.Address[3]=0x01;            
    for (int i=0;i<32;i++)
        pkt.Payload[i]=0;
    while(1)
    {                        
        ShowRegisters();                
        Timer.sleep(500);
        NRF905.transmit(&pkt);
        pkt.Payload[31]++;   // send a changing message to the receiver
    }
}

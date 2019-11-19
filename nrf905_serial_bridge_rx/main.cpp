// RX VERSION: This blinks an LED when a packet is received
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
uint8_t TXAddr[4]={0xAA,0xAA,0xAA,0x02};
uint8_t RXAddr[4]={0xAA,0xAA,0xAA,0x01};

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
    
    GPIOA->MODER &= ~(1 <<1);	// Make Port A, bit 0 behave as a
    GPIOA->MODER |= 1;	        // general purpose output
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
    
    uint32_t msg=0;
    uint16_t ch=0;
    setup();            
    NRF905.RXMode();
    serial.print("RX Modea\r\n");
    while(1)
    {                        
        if (NRF905.DataReady())
        {
            GPIOA->ODR |= 1;  // LED on
            uint8_t message[32];
            NRF905.readRXPayload(message,sizeof(message));
            serial.print("\r\nRX\r\n");
            for (int i=0;i < sizeof(message); i++)
            {
                serial.print(message[i]);
                serial.print(" ");
            }
            serial.print("\r\n");
            GPIOA->ODR &= ~1; // LED on
        }
        
        Timer.sleep(400);        
        
    }
}

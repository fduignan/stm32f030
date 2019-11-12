// C++ routines for the NRF905 wired to an STM32F030
#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "nrf905.h"
#include "spi.h"

int nrf905::begin(timer *pTimer, spi * SPI, uint8_t *rx_address, uint8_t *tx_address, uint32_t channel)

{
    RCC->AHBENR |= (1 << 18) + (1 << 17) + (1 << 22); 	// enable ports A,B and F
    // CE and CSN configuration 
/*     
 STM32F030          NRF905
 PA1                DR
 PA2                Pwr  ---> PF0
 PA3                CE   ---> PF1
 PA4                CSN
 PA5                SPI SCLK
 PA6                SPI MISO
 PA7                SPI MOSI
 PA9                UART TX  ---> PA2
 PA10               UART RX  ---> PA3
 
 ??                 CD
 PB1                TXEn
                    I2C SDA   PA10
                    I2C SCL   PA9 
*/    
  

    this->pTimer = pTimer; // Remember where the timer is for later
    this->pSPI = SPI; // Remember with the SPI object is
    GPIOF->MODER &= ~(1 <<1);	// Make Port F, bit 0 behave as a
    GPIOF->MODER |= 1;	// general purpose output
    
    GPIOF->MODER &= ~(1 << 3);	// Make Port F, bit 1 behave as a
    GPIOF->MODER |= (1 << 2);	// general purpose output

    GPIOA->MODER &= ~(1 << 9);  // Make Port A, bit 4 behave as a
    GPIOA->MODER |= (1 << 8);	// general purpose output

    GPIOB->MODER &= ~(1 << 3);   // Make PB1 an output
    GPIOB->MODER |= (1 << 2);
    
    
    CSNHigh(); 		// Make CSN output initially high
    CELow();	    // Make CE output initially low    
    TXEnLow();      // Listen mode
    PwrLow();       // Turn off the radio during initial programming
    // Need to set the XOF bits to reflect the fact that this module has a 16MHz crystal

    
    uint8_t R9 = readRegister(9);
    R9 = R9 & ~ ( (1 << 5) | (1 << 4) | (1 << 3));
    R9 = R9 | (1 << 4) | (1 << 3);
    writeRegister(9,R9);
    writeRegister(3,32);  // Set the RX Packet size 32 bytes
    writeRegister(4,32);  // Set the TX Packet size 32 bytes
    pTimer->sleep(3);
    return 0;
}

void nrf905::CELow()
{
    GPIOF->ODR &= ~(1 << 1);
    pTimer->sleep(1);
}
void nrf905::CEHigh()
{
    GPIOF->ODR |= (1 << 1);
    pTimer->sleep(1);
}
void nrf905::CSNLow()
{
    GPIOA->ODR &= ~(1 << 4);
    //pTimer->sleep(1);
}
void nrf905::CSNHigh()
{
    GPIOA->ODR |= (1 << 4);
 //   pTimer->sleep(1);
}
void nrf905::TXEnHigh()
{
    GPIOB->ODR |= (1 << 1);
    pTimer->sleep(2);
}
void nrf905::TXEnLow()
{
    GPIOB->ODR &= ~(1 << 1);
    pTimer->sleep(2);
}
void nrf905::PwrHigh()
{
    GPIOF->ODR |= 1;
    pTimer->sleep(3);
}
void nrf905::PwrLow()
{
    GPIOF->ODR &= ~1;
    pTimer->sleep(3);
}
uint8_t nrf905::readRegister(int RegNum)
{
    uint8_t ReturnValue;
    CSNLow();
    ReturnValue = pSPI->transferSPI((uint8_t) (0b00010000+RegNum));        
    ReturnValue = pSPI->transferSPI((uint8_t) (0));        
    CSNHigh();
    return ReturnValue;
}
void nrf905::writeRegister(int RegNum, uint8_t contents)
{
    uint16_t TXWord;
// Need to organize the outbound 16 bit number in reverse order little/big endian mixup    
    TXWord = contents;
    TXWord = TXWord << 8;
    TXWord += RegNum;    
    CSNLow();
    pSPI->transferSPI(TXWord);    
    CSNHigh();    
}
uint8_t nrf905::getStatus()
{
    // Address Match : Bit 7
    // Data ready : Bit 5    
    uint8_t ReturnValue;
    CSNLow();
    ReturnValue = pSPI->transferSPI((uint8_t) (0b00010000));        
    CSNHigh();
    return ReturnValue;
}
void nrf905::readRXAddress(uint8_t *addr)
{
    addr[0] = readRegister(5);
    addr[1] = readRegister(6);
    addr[2] = readRegister(7);
    addr[3] = readRegister(8);
}
void nrf905::setRXAddress(const uint8_t *addr)
{
    writeRegister(5,addr[0]);
    writeRegister(6,addr[1]);
    writeRegister(7,addr[2]);
    writeRegister(8,addr[3]);
    
}
void nrf905::readTXAddress(uint8_t *addr)
{
    CSNLow();
    pSPI->transferSPI((uint8_t)0b00100011);
    addr[0]=pSPI->transferSPI((uint8_t)0);
    addr[1]=pSPI->transferSPI((uint8_t)0);
    addr[2]=pSPI->transferSPI((uint8_t)0);
    addr[3]=pSPI->transferSPI((uint8_t)0);
    CSNHigh();
}
void nrf905::setTXAddress(const uint8_t *addr)
{
    CSNLow();
    pSPI->transferSPI((uint8_t)0b00100010);
    pSPI->transferSPI((uint8_t)addr[0]);
    pSPI->transferSPI((uint8_t)addr[1]);
    pSPI->transferSPI((uint8_t)addr[2]);
    pSPI->transferSPI((uint8_t)addr[3]);
    CSNHigh();
}
void nrf905::setChannel(int Channel)
{
    // Channel is 9 bits long and is spread over 2 registers
    // R0 contains bits 0 to 7
    // R1 LSB = bit8
    writeRegister(0,Channel & 0xff);
    uint8_t R1 = readRegister(1);
    if (Channel & 0x100)
        R1 = R1 | 1;
    else
        R1 = R1 & ~1;    
    writeRegister(1,R1);            
}
void nrf905::setBand(int Band)
{
    // 0 = 433 MHz band
    // 1 = 868 MHz band
    // This bit is stored in bit 1 of R1
    uint8_t R1 = readRegister(1);
    if (Band == 0)
        R1 = R1 & ~2;
    if (Band == 1)
        R1 = R1 | 2;
    writeRegister(1,R1);
        
}
void nrf905::setAutoRetran(int Auto)
{
    // Controls whether device will retyr transmissions
    // This is stored in bit 5 of R1
    uint8_t R1 = readRegister(1);
    if (Auto == 0)
        R1 = R1 & ~(1 << 5);
    if (Auto == 1)
        R1 = R1 | (1 << 5);
    writeRegister(1,R1);

}
void nrf905::setTXPower(int pwr)
{
    // This is stored in bits 3:2 of R1
    // 00 : -10dBm
    // 01 : -2dBm
    // 10 : +6dBm
    // 11 : +10dBm
   uint8_t R1 = readRegister(1);
   R1 = R1 & ~( (1 << 3) | (1 << 2)); // clear old bits
   pwr = pwr & 3; // restrict to 2 bit value 
   R1 = R1 | (pwr << 2); // set new bits
   writeRegister(1,R1); // write to NRF

    
}
void nrf905::setRXLowPower(int pwr)
{
    // This is stored in Bit 4 of R1
    // If this bit is a 1 then the RX circuit consumes less power (with reduced senitivity)
    
    uint8_t R1 = readRegister(1);
    if (pwr == 1)
        R1 = R1 | (1 << 4);
    else
        R1 = R1 & ~(1 << 4);
   
   writeRegister(1,R1); // write to NRF
    
    
}
void nrf905::setRXAFW(int afw)
{
    // stored in bits 2:0 of R2
    uint8_t R2 = readRegister(2);
    R2 = R2 & 0xf8; // clear old bits
    if (afw == 1)
    {
        R2 = R2 | (1);
        writeRegister(2,R2); // write to NRF
    }
    if (afw == 4)
    {
        R2 = R2 | 4;
        writeRegister(2,R2); // write to NRF
    }
   
   
}
void nrf905::setTXAFW(int afw)
{
    // stored in bits 2:0 of R2
    uint8_t R2 = readRegister(2);
    R2 = R2 & 0x8f; // clear old bits
    if (afw == 1)
    {
        R2 = R2 | (0x10);
        writeRegister(2,R2); // write to NRF
    }
    if (afw == 4)
    {
        R2 = R2 | 0x40;
        writeRegister(2,R2); // write to NRF
    }
}

void nrf905::writeTXPayload(uint8_t *Payload, unsigned length)
{
    if (length <= 32) {
        writeRegister(4,length);
        CSNLow();    
        pSPI->transferSPI((uint8_t)0b00100000);
        while(length--)
            pSPI->transferSPI(*Payload++);
        CSNHigh();    
    }
}
void nrf905::readTXPayload(uint8_t *Payload, unsigned length)
{
    CSNLow();
    pSPI->transferSPI((uint8_t)0b00100001);
    while(length--)
        *Payload++ = pSPI->transferSPI((uint8_t)0);
    CSNHigh();    
}
void nrf905::readRXPayload(uint8_t *Payload, unsigned length)
{
    CSNLow();
    pSPI->transferSPI((uint8_t)0b00100100);
    while(length--)
        *Payload++ = pSPI->transferSPI((uint8_t)0);
    CSNHigh();    
}
int nrf905::DataReady()
{
    /*uint8_t status = getStatus();
    if (status & (1 << 5))
        return 1;
    else
        return 0;*/
    if (GPIOA->IDR & (1 << 1) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void nrf905::transmit(NRF905DataPacket *Pkt)
{
    
    CELow();
    TXEnLow();        
    PwrHigh();
    writeRegister(4,PAYLOAD_LENGTH);  // Set the TX Packet size 32 bytes    
    CSNLow();
    pSPI->transferSPI((uint8_t)0b00100010);
    pSPI->transferSPI(Pkt->Address[0]);
    pSPI->transferSPI(Pkt->Address[1]);
    pSPI->transferSPI(Pkt->Address[2]);
    pSPI->transferSPI(Pkt->Address[3]);
    CSNHigh();
    CSNLow();
    pSPI->transferSPI((uint8_t)0b00100000);
    for (int i=0;i < PAYLOAD_LENGTH ; i++)
        pSPI->transferSPI(Pkt->Payload[i]);    
    CSNHigh();
    CEHigh();
    TXEnHigh();
    pTimer->sleep(20);      
    TXEnLow();
    CELow();   
   
}

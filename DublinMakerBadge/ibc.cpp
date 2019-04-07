#include "ibc.h"
#include <stdint.h>
#include "../include/STM32F0x0.h"
void ibc::begin()
{
    InputIndex = 0;
    OutputIndex = 0;
    PacketReceived = 0;
    PacketWaiting = 0;
    MyAddress = UNASSIGNED_ADDRESS;
    // Turn on the clock for GPIOA (usart 1 uses it) - not sure if I need this
    RCC->AHBENR |= (1 << 17);
// Turn on the clock for the USART1 peripheral	
    RCC->APB2ENR |= (1 << 14);
// De-assert reset of USART1 
    RCC->APB2RSTR &= ~(1 << 14);

    USART1->CR2 = 0;
    USART1->CR3 = (1 << 3); // enable half duplex operation

// Set the baud rate
	USART1->BRR = 48000000/9600;    
    
    
// Make PA9 and PA10 an open collector Output 
    GPIOA->OTYPER |= ( (1 << 9) + (1 << 10) );
// Turn on the pull-up resistors for PA9 and PA10
    GPIOA->PUPDR |= ( (1 << 18) + (1 << 20) );
    GPIOA->PUPDR &= ~( (1 << 19) + (1 << 21) );
// assign alternate function to PA9 and output to PA10
    GPIOA->MODER |= (1 << 19)+(1<<20);
    GPIOA->MODER &= ~((1 << 18) + (1 << 21));
    deassertLineBusy();
    USART1->CR1 = (1 << 2) + (1 << 0); // enable the transmitter and he USART

    announce();
}
int ibc::sendPacket(uint8_t Destination,uint8_t Flags,uint8_t len,uint8_t *Packet)
{
    uint8_t Index=0;
    if (len < MAX_PAYLOAD_SIZE)
    {
        OutputBuffer[Index++] = Destination;
        OutputBuffer[Index++] = MyAddress;
        OutputBuffer[Index++] = Flags | len;
        while(len--)
        {
            OutputBuffer[Index++] = *Packet++;
        }
        OutputBuffer[Index] = getCHK(Index-1,OutputBuffer);
        OutputPacketLength = Index;
        PacketWaiting = 1;
    }
}
int ibc::packetAvailable() 
{
}
int ibc::getPacket(uint8_t len, uint8_t *Packet)
{
    
}
uint8_t ibc::getCHK(uint8_t len,uint8_t *Packet)
{
    return 1; 
}
uint8_t ibc::validatePacket(uint8_t len, uint8_t *Packet)
{
    // Assumption : last byte in packet is the checksum
    uint8_t crc=getCHK(len-1,Packet);
    if (crc == Packet[len-1])
    {
        return 1; // valid packet
    }
    else
    {
        return 0; // invalid
    }
}
void ibc::assertLineBusy()
{
     GPIOA->ODR &= ~(1 << 10);
}
void ibc::deassertLineBusy()
{
    GPIOA->ODR |= (1 << 10);
}
void ibc::OnLineBecomesFree()
{
    if (InputIndex > 0)
    {
        // Some data was received.  Try to validate it as a packet
        if (validatePacket(InputIndex,InputBuffer))
        {
            // A valid packet was received
            if (InputBuffer[0]==BROADCAST_ADDRESS)
            {
                // Got a broadcast packet, check the flags field
                if (InputBuffer[2] & ANNOUNCE_FLAG)
                {
                    // It's an announce frame
                    if (InputBuffer[1]==MyAddress)
                    {
                        // I already have this address
                        sendNAK();
                    }
                }
                
            }
            else if (InputBuffer[0]==MyAddress)
            {
                // it's a valid packet addressed to me
                PacketWaiting = 1;
            }
        }
    }
    if (PacketWaiting)
    {
        // Line is free so send a pending packet
        assertLineBusy();
        // Will not do interrupt driven transmission for now - polled is fine
        while (OutputIndex < OutputPacketLength)
        {
            USART1->TDR = OutputBuffer[OutputIndex++];
            while((USART1->ISR & (1 << 6))==0); // wait for transmission to complete
        }
        deassertLineBusy();
    }
}
void ibc::OnLineBecomesBusy()
{
    // clear out the packet buffer, a new packet is incoming
    for (InputIndex=0;InputIndex < MAX_PACKET_SIZE;InputIndex++)
    {
        InputBuffer[InputIndex] = 0;
    }
    InputIndex = 0;
    PacketReceived = 0;
}
void ibc::sendNAK()
{
    uint8_t NAK_PACKET[5];
    uint8_t index=0;
    NAK_PACKET[0] = BROADCAST_ADDRESS;
    NAK_PACKET[1] = MyAddress;
    NAK_PACKET[2] = NAK_FLAG;
    NAK_PACKET[3] = getCHK(3,NAK_PACKET);
    assertLineBusy();
    while (index < 4)
    {
        USART1->TDR = NAK_PACKET[index++];
        while((USART1->ISR & (1 << 6))==0); // wait for transmission to complete
    }
    
    deassertLineBusy();
}
void ibc::announce()
{
    uint8_t AnnoucePacket[4];
    uint8_t index;
    AnnoucePacket[0] = BROADCAST_ADDRESS;
    AnnoucePacket[2] = ANNOUNCE_FLAG;
    MyAddress = BROADCAST_ADDRESS-1;
    while (MyAddress)
    {
        AnnoucePacket[1] = MyAddress;
        AnnoucePacket[3] = getCHK(3,AnnoucePacket);
        sendPacket(BROADCAST_ADDRESS,ANNOUNCE_FLAG,0,AnnoucePacket);
        // How do I get the NAK back?
    }
}

#include "ibc.h"
#include "timer.h"
#include <stdint.h>
#include "../include/STM32F0x0.h"
static ibc * pibc;
void ibc::begin(timer *t)
{
    pibc = this;
    Timer = t;  // Don't use timer in interrupt context! i.e. do not use in OnLineBecomesBusy or OnLineBecomesFree or in the handle_rxdata function
    InputIndex = 0;    
    RXPacketWaiting = 0;
    TXPacketWaiting = 0;    
    State = IdleState;
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
	USART1->BRR = 48000000/LINE_SPEED;    
    
    
// Make PA9 and PA10 an open collector Output 
    GPIOA->OTYPER |= ( (1 << 9) + (1 << 10) );
// Turn on the pull-up resistors for PA9 and PA10
    GPIOA->PUPDR |= ( (1 << 18) + (1 << 20) );
    GPIOA->PUPDR &= ~( (1 << 19) + (1 << 21) );
// assign alternate function to PA9 and output to PA10
    GPIOA->MODER |= (1 << 19)+(1<<20);
    GPIOA->MODER &= ~((1 << 18) + (1 << 21));
    GPIOA->AFRH &= ~(0xf0); // PA9 -> Serial TX = Alternate function 1
    GPIOA->AFRH |= (1 << 4);
    deassertLineBusy();
    USART1->CR1 = (1 << 5) + (1 << 3) + (1 << 2) + (1 << 0); // USART, receiver, transmitter, and reciever interrupts
	NVIC->ISER |= (1<<27); // enable interrupts in NVIC

// Enabling interrupts on PA10
    // PA10 is on EXIT10
    EXTI->IMR |= (1 << 10);
    EXTI->RTSR |= (1 << 10);
    EXTI->FTSR |= (1 << 10);
    NVIC->ISER |= (1 << 7);
    
    announce();
}
int ibc::sendPacket(uint8_t Destination,uint8_t Flags,uint8_t len,uint8_t *Packet)
{
    uint8_t Index=0;    
    uint8_t *Pkt=Packet;
    if (len < MAX_PAYLOAD_SIZE)
    {
        OutputBuffer[Index++] = Destination;
        OutputBuffer[Index++] = MyAddress;
        OutputBuffer[Index++] = Flags | len;
        while(len--)
        {
            OutputBuffer[Index++] = *Pkt++;
        }
        OutputBuffer[Index] = getCHK(Index,OutputBuffer);
        OutputPacketLength = Index+1;
        TXPacketWaiting = 1;
    }
    if (lineIsFree()) // if line is available then send now
    {
        OnLineBecomesFree();
    }
    return 0;
}

int ibc::getPacket(uint8_t len, uint8_t *Packet)
{
    return 0;
}
uint8_t ibc::getCHK(uint8_t len,uint8_t *Packet)
{
    uint8_t sum=0;
    while(len--)
    {
        sum += *Packet++;
    }
    return sum; 
}
uint8_t ibc::validatePacket(uint8_t len, uint8_t *Packet)
{
    return 1;
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
int ibc::lineIsFree()
{
    if (GPIOA->IDR & (1 << 10))
        return 1;
    else
        return 0;
}
void ibc::OnLineBecomesFree()
{
    if (State == AnnouncingState)
        return;
    if (InputIndex > 0)
    {
        State=ParsingState;
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
                        // I have this address                       
                        sendNAK();
                        InputIndex = 0;
                    }
                }
                // Got a broadcast packet, check the flags field
                if (InputBuffer[2] & NAK_FLAG)
                {
                    // It's a NAK signal.
                }
            }
            else if (InputBuffer[0]==MyAddress)
            {                
                // it's a valid packet addressed to me
                GPIOA->ODR |= (1<<3);   
                RXPacketWaiting = 1;
            }
        }
        State=IdleState;
    }
    if (TXPacketWaiting)
    {
        State=SendingState;
        // Line is free so send a pending packet
        assertLineBusy();
        // Will not do interrupt driven transmission for now - polled is fine
        uint32_t OutputIndex = 0;
        while (OutputIndex < OutputPacketLength)
        {
            USART1->TDR = OutputBuffer[OutputIndex++];
            while((USART1->ISR & (1 << 6))==0); // wait for transmission to complete
        }        
        TXPacketWaiting = 0;
        deassertLineBusy();        
        State=IdleState;
    }
}
void ibc::OnLineBecomesBusy()
{
    if (State != SendingState) 
    {            
        // prepare to receive data        
        InputIndex = 0;
        RXPacketWaiting = 0;        
    }    

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
    uint8_t AnnouncePacket[4];
    uint8_t index;
    State=AnnouncingState;
    RXPacketWaiting = 0;
    AnnouncePacket[0] = BROADCAST_ADDRESS;
    AnnouncePacket[2] = ANNOUNCE_FLAG;
    MyAddress = BROADCAST_ADDRESS-1;
    while (MyAddress)
    {        
        AnnouncePacket[1] = MyAddress;                
        AnnouncePacket[3] = getCHK(3,AnnouncePacket);
               
        int Timeout=LINE_IDLE_TIMEOUT; // wait for the line to become free.
        while (lineIsFree()==0)
        {
            Timeout--;
            Timer->sleep(1);
        }                        
        if (!Timeout)
        {
            // Line is permanently busy - must be some kind of fault
            MyAddress = 0; // not a valid address 
            break;
        }
        
        assertLineBusy();
        index = 0;                
        while (index < 4)
        {   
            USART1->TDR = AnnouncePacket[index++];
            while((USART1->ISR & (1 << 6))==0); // wait for transmission to complete
        }        
        deassertLineBusy();                                      
        InputIndex=0;
        Timeout=20; // wait up to 20 byte periods for a NAK
        while (Timeout && (InputIndex < 4) ) 
        {
            Timeout--;
            Timer->sleep(1+(10000/LINE_SPEED)); 
        }
        if (InputIndex>=4)
        {            
            // Did we get a NAK
            if (InputBuffer[2] & NAK_FLAG)            
                MyAddress--;              
        }
        else
        {
            // NAK not received so exit with this address.
            
            break;
        }        
    }
    State=IdleState;
}


void ibc::handle_rxdata()
{
    uint8_t rxdata;
    rxdata = USART1->RDR;     
    USART1->ICR = 0x6010d8;
    if (State != SendingState) { // Don't buffer our own outbound data 
        if (InputIndex < MAX_PACKET_SIZE)
        {
            InputBuffer[InputIndex]=rxdata;
            InputIndex++;
        }
    }
}
void USART1_Handler(void)
{
	if (USART1->ISR & (1 << 5)) // is it an RXNE interrupt?
		pibc->handle_rxdata();
    
}
void Line_State_Handler(void)
{
// This interrupt handler is called when the Line_State line changes state.  It 
// then calls on methods within the ibc class to handle the event further
    EXTI->PR = (1 << 10); // clear pending interrupt
    if (GPIOA->IDR & (1 << 10))
    {
        // must be a rising edge                
        pibc->OnLineBecomesFree();
    }
    else
    {
        // must be a falling edge        
        pibc->OnLineBecomesBusy();
    }
}

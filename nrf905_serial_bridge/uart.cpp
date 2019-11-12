#include "uart.h"
#include "../include/STM32F0x0.h"
static uart *uartptr;
int uart::begin()
{
    
    uartptr = this;
    // Turn on the clock for GPIOA (usart 1 uses it) - not sure if I need this
    RCC->AHBENR  |= (1 << 17);
// Turn on the clock for the USART1 peripheral    
    RCC->APB2ENR |= (1 << 14);

    
// Configure the I/O pins.  Will use PA3 as RX and PA2 as TX
    GPIOA->MODER |= ((1 << 5) | (1 << 7));
    GPIOA->MODER &= ~(1 << 4);
    GPIOA->MODER &= ~(1 << 6);
// The alternate function number for PA2 and PA3 is AF1 (see datasheet, reference manual)
    GPIOA->AFRL  &= (0xffff00ff);
    GPIOA->AFRL  |= (1 << 8);
    GPIOA->AFRL  |= (1 << 12);    
    
    uint32_t BaudRateDivisor = 48000000;  // assuming 8MHz clock 
    BaudRateDivisor = BaudRateDivisor / (long) 9600;

// De-assert reset of USART1 
    RCC->APB2RSTR &= ~(1 << 14);
// Configure the USART
// disable USART first to allow setting of other control bits
// This also disables parity checking and enables 16 times oversampling

    USART1->CR1 = 0; 
 
// Don't want anything from CR2
    USART1->CR2 = 0;

// Don't want anything from CR3
    USART1->CR3 = 0;

// Set the baud rate
    USART1->BRR = BaudRateDivisor;

// Turn on Transmitter, Receiver, Transmit and Receive interrupts.    
    USART1->CR1 |= ((1 << 2)  | (1 << 5) | (1 << 6)); 
// Enable the USART
    USART1->CR1 |= 1;
// Enable USART1 interrupts in NVIC    
    NVIC->ISER |= (1 << 27);
// and enable interrupts 
    enable_interrupts();    
    return 0;
}

uint8_t uart::egetc() 
{
    if (rx_buffer.get_count())
        return rx_buffer.get();
    else        
        return 0;
}
void uart::eputc(uint8_t b) 
{
    while(tx_buffer.is_full());
    tx_buffer.put(b);
    if ( (USART1->CR1 & (1 << 3))==0)
    { // transmitter was idle, turn it on and force out first character
        USART1->CR1 |= (1 << 3);
        USART1->TDR = tx_buffer.get();
    } 
}
void uart::print(const char *s)
{
    while(*s)
        eputc(*s++);
}
void uart::print(uint8_t b)
{
    char buffer[3];
    buffer[2] = 0; //terminate the string
    buffer[1] = (b & 0x0f);
    if (buffer[1] < 10)
        buffer[1] += 48;
    else
        buffer[1] += 55;
    b = b >> 4;
    buffer[0] = b;
    if (buffer[0] < 10)
        buffer[0] += 48;
    else
        buffer[0] += 55;
    this->print(buffer);
}
void uart::print(uint16_t w)
{
    uint8_t b;
    b = w >> 8;
    this->print(b);
    b = w & 0xff;
    this->print(b);
}
void uart::print(uint32_t l)
{
    uint16_t w;
    w = l >> 16;
    this->print(w);
    w = l & 0xffff;
    this->print(w);
}
void uart::handle_tx()
{
    if (tx_buffer.get_count())
    { 
        USART1->TDR = tx_buffer.get();
    }
    else
    {
 // No more data, disable the transmitter 
        USART1->CR1 &= ~(1 << 3);
        if (USART1->ISR & (1 << 6))
        // Write TCCF to USART_ICR
        USART1->ICR |= (1 << 6);
        if (USART1->ISR & (1 << 7))
        // Write TXFRQ to USART_RQR
        USART1->RQR |= (1 << 4);            
    }
}
void uart::handle_rx()
{
    // Handles serial comms reception
    // simply puts the data into the buffer 
    // If the buffer is full data is lost
    rx_buffer.put(USART1->RDR);

}
void USART1_Handler(void)
{
// check which interrupt happened.
      if (USART1->ISR & (1 << 7)) // is it a TXE interrupt?
        uartptr->handle_tx();
    if (USART1->ISR & (1 << 5)) // is it an RXNE interrupt?
        uartptr->handle_rx();

}
int circular_buffer::put(uint8_t b)
{
    if ( (head==tail) && (count!=0))
        return(1);  /* OverFlow */
    disable_interrupts();
    buffer[head++] = b;
    count++;
    if (head==MAXBUFFER)
        head=0;
    enable_interrupts();
    return(0);
};
uint8_t circular_buffer::get()
{
    uint8_t Data;
    if ( count==0 )
        return (0);
    disable_interrupts();
    Data = buffer[tail++];
    if (tail == MAXBUFFER)
        tail = 0;
    count--;
    enable_interrupts();
    return (Data);
};

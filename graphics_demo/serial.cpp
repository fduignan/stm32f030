#include <stdint.h>
#include "serial.h"
#include "../include/STM32F0x0.h" 

static serial *pSerial;  // need a pointer to the hopefully only instance of Serial for the interrupt service routine
void serial::begin()
{
    int BaudRateDivisor;
    int BaudRate = 9600;
    pSerial = this;
	disable_interrupts();	
// Turn on the clock for GPIOA (usart 2 uses it)
	RCC->AHBENR  |= (1 << 17);
// Turn on the clock for the USART1 peripheral	
	RCC->APB2ENR |= (1 << 14);

	
// Configure the I/O pins.  Will use PA9 as RX and PA10 as TX
	GPIOA->MODER |= ((1 << 19) | (1 << 21));
	GPIOA->MODER &= ~(1 << 20);
	GPIOA->MODER &= ~(1 << 18);
// The alternate function number for PA9 and PA10 is AF1 (see datasheet, reference manual)
	GPIOA->AFRH  &= ~((1 << 11) | (1 << 10) | (1 << 19) | (1 << 7) | (1 << 6) | (1 << 5));
	GPIOA->AFRH  |= (1 << 8);
	GPIOA->AFRH  |= (1 << 4);	 
	
	BaudRateDivisor = 48000000; // assuming 8MHz APB1 clock 
	BaudRateDivisor = BaudRateDivisor / (long) BaudRate;

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
}
void serial::eputc(char c)
{
	if ((USART1->CR1 & (1 << 3))==0)
	{ // transmitter was idle, turn it on and force out first character
	  USART1->CR1 |= (1 << 3);      
	  USART1->TDR = c;
	} 
    else
    {   // interrupt transmission is underway so add byte to the queue
        TXBuffer.putBuf(c);
    }
}
char serial::egetc()
{
    // return next character in buffer (0 if empty)
    return RXBuffer.getBuf();
}
void serial::printHex(uint32_t Value)
{
        // Convert Value to a string equivalent and call the 
    // print(string) version of this function
    
    char Buffer[9]; // ints can be +/- 2 billion (approx) so buffer needs to be this big
    int index=7;    // Start filling buffer from the right
    Buffer[8]=0;    // ensure the buffer is terminated with a null    
    while(index >= 0)  // go through all of the characters in the buffer
    {
        char c = Value % 16;
        if (c > 9)
            c += 55;
        else 
            c += 48;
        Buffer[index]=c;        
        Value = Value >> 4 ; // move on to the next digit
        index--;  
    }
    print(Buffer); // call string version of print
}
void serial::printHex(uint8_t Value)
{
        // Convert Value to a string equivalent and call the 
    // print(string) version of this function
    
    char Buffer[3]; 
    int index=1;    // Start filling buffer from the right
    Buffer[2]=0;    // ensure the buffer is terminated with a null    
    while(index >= 0)  // go through all of the characters in the buffer
    {
        char c = Value % 16;
        if (c > 9)
            c += 55;
        else 
            c += 48;
        Buffer[index]=c;        
        Value = Value >> 4 ; // move on to the next digit
        index--;  
    }
    print(Buffer); // call string version of print
}
void serial::print(int Value)
{
    // Convert Value to a string equivalent and call the 
    // print(string) version of this function
    
    char Buffer[12]; // ints can be +/- 2 billion (approx) so buffer needs to be this big
    int index=10;    // Start filling buffer from the right
    Buffer[11]=0;    // ensure the buffer is terminated with a null
    if (Value < 0)  
    {
        Buffer[0]='-';  // insert a leading minus sign
        Value = -Value; // make Value positive
    }
    else
    {
        Buffer[0]='+';  // insert a leading plus sign
    }
    while(index > 0)  // go through all of the characters in the buffer
    {
        Buffer[index]=(Value % 10)+'0'; // convert the number to ASCII
        Value = Value / 10; // move on to the next digit
        index--;  
    }
    print(Buffer); // call string version of print
}
void serial::print(const char *String)
{
    while(*String) // keep going until a null is encountered
    {
        while(TXBuffer.isFull());
        eputc(*String); // write out the next character
        String++;       // move the pointer along
    }
}

void serial::handle_rx()
{
// Handles serial comms reception
// simply puts the data into the buffer.  Does not report error if buffer is full    
	RXBuffer.putBuf(USART1->RDR);    
}
	
void serial::handle_tx()
{
    
// Handles serial comms transmission
// When the transmitter is idle, this is called and the next byte
// is sent (if there is one)
	if (TXBuffer.getBufCount())
		USART1->TDR=TXBuffer.getBuf();
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
// This is an interrupt handler for serial communications.  
void USART1_Handler(void)
{        
    // check which interrupt happened.
    if (USART1->ISR & (1 << 7)) // is it a TXE interrupt?
		pSerial->handle_tx();
	if (USART1->ISR & (1 << 5)) // is it an RXNE interrupt?
		pSerial->handle_rx();
}
int circular_buffer::putBuf(char c)
{
    if (count < SBUFSIZE)   
    {
        disable_interrupts();
        count++;
        buffer[head] = c;
        head=(head+1) % SBUFSIZE;
        enable_interrupts();
        return 0;
    }
    return -1;
}
int circular_buffer::getBuf()
{
    char c=0;
    if (count >0 )
    {
        disable_interrupts();
        count--;
        c=buffer[tail];
        tail=(tail+1) % SBUFSIZE;
        enable_interrupts();        
    }
    return c;    
}

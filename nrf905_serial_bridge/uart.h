#ifndef __uart_h
#define __uart_h
#include <stdint.h>
#define MAXBUFFER 32
class circular_buffer {
public:
    circular_buffer() {
        head = tail = count = 0;
    }
    int put(uint8_t b);
    uint8_t get();
    uint32_t get_count()  
    {
        return count;
    }
    int is_full() {
        if (count >= MAXBUFFER)
            return 1;
        else
            return 0;
    }
private:    
    volatile uint32_t head, tail, count;
    uint8_t buffer[MAXBUFFER];    
};
class uart {
public:
    uart() {};
    int begin();
    void eputc(uint8_t b);
    uint8_t egetc();
    void print(const char *s);
    void print(uint8_t b);
    void print(uint16_t w);
    void print(uint32_t l);

    uint32_t available();
private:
#define BUF_SIZE 32
    void handle_tx();
    void handle_rx();
    circular_buffer tx_buffer;
    circular_buffer rx_buffer;
    friend void USART1_Handler(void);            
};
#endif

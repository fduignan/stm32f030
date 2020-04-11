#ifndef __serial_h
#define __serial_h
class circular_buffer {
#define SBUFSIZE 16
public:    
    circular_buffer()
    {
        count=head=tail=0;
    }
    int getBufCount()
    {
        return count;
    }
    int isFull()
    {
        if (count==SBUFSIZE)
            return 1;
        else
            return 0;
    }
    int putBuf(char c);
    int getBuf();
private:    

    volatile unsigned head,tail,count;
	char buffer[SBUFSIZE];
};

class serial {
public:
    serial() {};
    void begin();
    void print(int Value);
    void print(const char *Str);
    void printHex(uint32_t Value);
    void printHex(uint8_t Value);
    void eputc(char ch);
    char egetc();
    void handle_rx();
    void handle_tx();
private:    
    circular_buffer TXBuffer,RXBuffer;
    friend void USART1_Handler(void);    
};
#endif

// main.c

#include "serial.h"
#include <reg52.h>
#include <stdio.h>
#include <stdlib.h>

void delay10ms(int count)   //误差 0us
{
    unsigned char i, j;
    
    //--c已经在传递过来的时候已经赋值了，所以在for语句第一句就不用赋值了--//
    while (--count >= 0) {
        for (i = 0; i < 38; i++) {
            for (j = 0; j < 130; j++)
                ;
        }
    }
}

void main_parse(struct serial *sender)
{
    unsigned short i;
    
    while (1) {
        for (i = 0; i < sender->rfifo_size - sender->rfifo_pos; i++) {
            if (sender->rfifo[sender->rfifo_pos+i] == '\n')
                break;
        }
        if (i == sender->rfifo_size - sender->rfifo_pos) break;
        sender->rfifo[sender->rfifo_pos+i] = 0;
        serial_send(&sender->rfifo[sender->rfifo_pos]);
        sender->rfifo_pos += i + 1;
        TI = 1;
        printf(" pos: %u, size: %u", sender->rfifo_pos, sender->rfifo_size);
        while (TI == 0);
        TI = 0;
        serial_send_char('\n');
    }
}

int main()
{
    serial_init(main_parse);
    serial_config(F12B4800);
    
    while (1) {
        serial_perform();
        delay10ms(1);
    }
    
    return 0;
}
#ifndef _SERIAL_H_
#define _SERIAL_H_

#define RFIFO_LIMIT 128

typedef void (*RFIFO_PARSE_FUNC)(struct serial *sender);

struct serial {
    char *rfifo;
    unsigned short rfifo_max;
    volatile unsigned short rfifo_size;
    unsigned short rfifo_pos;
    
    RFIFO_PARSE_FUNC rfifo_parse;
};

enum serial_type {
    F11B9600 = 0,
    F12B4800 = 1,
};

extern void serial_init(RFIFO_PARSE_FUNC parse);
/**
 * type = 0 => fos = 11.0592, baudrate = 9600
 * type = 1 => fos = 12, baudrate = 4800
 */
extern void serial_config(int type);
extern void serial_perform();
extern void serial_send(char *str);
extern void serial_send_char(char c);

extern struct serial *seri; // init by serial_init

#endif
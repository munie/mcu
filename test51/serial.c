// serial.c

#include "serial.h"
#include <reg52.h>
#include <stdio.h>
#include <string.h>

struct serial *seri;
static struct serial _serial;
static char rfifo[RFIFO_LIMIT+1];

void serial_init(RFIFO_PARSE_FUNC parse)
{
    seri = &_serial;
    seri->rfifo = rfifo;
    seri->rfifo_max = RFIFO_LIMIT;
    seri->rfifo_size = 0;
    seri->rfifo_pos = 0;
    seri->rfifo_parse = parse;
}

void serial_config(int type)
{
    TMOD = 0x20;
    if (type == F11B9600) {
        TL1 = 0xfd;
        TH1 = 0xfd;
    }
    else if (type == F12B4800) {
        PCON = 0x80;
        TL1 = 0xf3;
        TH1 = 0xf3;
    }
    TR1 = 1;

    SM0 = 0;
    SM1 = 1;
    REN = 1;
    ES = 1;
    EA  = 1;
}

void serial_perform()
{
    int i, size, pos;

    // 没有收到任何数据，直接返回
    if (seri->rfifo_size == seri->rfifo_pos)
        return;

    // 性能考虑，不屏蔽ES，所以需记录下当前size。
    // 当前pos无需记录的，不过可以缩短代码
    size = seri->rfifo_size;
    pos = seri->rfifo_pos;

    // 如果未找到结束符'\n'，直接返回
    for (i = 0; i < size - pos; i++) {
        if (seri->rfifo[pos+i] == '\n')
            break;
    }
    if (i == size - pos) {
        // 缓冲区溢出，重置之
        if (size == RFIFO_LIMIT)
            seri->rfifo_size = seri->rfifo_pos = 0;
        return;
    }

    // 收到结束符，调用外部函数处理之
    ES = 0;
    if (seri->rfifo_parse)
        seri->rfifo_parse(seri);
    if (seri->rfifo_size == seri->rfifo_pos) {
        seri->rfifo_size = seri->rfifo_pos = 0;
    }
    else {
        seri->rfifo_size -= seri->rfifo_pos;
        memmove(seri->rfifo, &seri->rfifo[seri->rfifo_pos], seri->rfifo_size);
        seri->rfifo_pos = 0;
    }
    ES = 1;
}

void serial_send(char *str)
{
    char es_bak = ES;

    ES = 0;
    TI = 1;
    printf(str);
    while (TI == 0);
    TI = 0;
    ES = es_bak;
}

void serial_send_char(char c)
{
    char es_bak = ES;

    ES = 0;
    SBUF = c;
    while (TI == 0);
    TI = 0;
    ES = es_bak;
}

static void serial_interrupt() interrupt 4
{
    if (seri->rfifo_size != RFIFO_LIMIT) {
        seri->rfifo[seri->rfifo_size] = SBUF;
        seri->rfifo_size++;
    }

    RI = 0;
}

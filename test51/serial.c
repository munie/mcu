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

    // û���յ��κ����ݣ�ֱ�ӷ���
    if (seri->rfifo_size == seri->rfifo_pos)
        return;

    // ���ܿ��ǣ�������ES���������¼�µ�ǰsize��
    // ��ǰpos�����¼�ģ������������̴���
    size = seri->rfifo_size;
    pos = seri->rfifo_pos;

    // ���δ�ҵ�������'\n'��ֱ�ӷ���
    for (i = 0; i < size - pos; i++) {
        if (seri->rfifo[pos+i] == '\n')
            break;
    }
    if (i == size - pos) {
        // ���������������֮
        if (size == RFIFO_LIMIT)
            seri->rfifo_size = seri->rfifo_pos = 0;
        return;
    }

    // �յ��������������ⲿ��������֮
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

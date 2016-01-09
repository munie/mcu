// watchdog.c

#include "watchdog.h"
#include "stm32f10x.h"

void watchdog_init(unsigned char prer, unsigned int reld)
{
    //Tout=64��6250/40=10000mS=10S
    //���Ź���ʼ����������prer-��Ƶ��reld-��������װ��ֵ
    IWDG->KR = 0x5555; //�������PR��RLR�Ĵ���
    IWDG->PR = prer;  //���÷�Ƶ
    IWDG->RLR = reld; //�趨��������ֵ
    IWDG->KR = 0xaaaa; //����װ��ֵ
    IWDG->KR = 0xcccc;  //�������Ź���ʱ��
}

void watchdog_feed(void)
{
    IWDG->KR=0xaaaa;
}

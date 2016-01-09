// watchdog.c

#include "watchdog.h"
#include "stm32f10x.h"

void watchdog_init(unsigned char prer, unsigned int reld)
{
    //Tout=64×6250/40=10000mS=10S
    //看门狗初始化，参数：prer-分频，reld-计数器重装载值
    IWDG->KR = 0x5555; //允许访问PR和RLR寄存器
    IWDG->PR = prer;  //设置分频
    IWDG->RLR = reld; //设定计数器初值
    IWDG->KR = 0xaaaa; //初次装初值
    IWDG->KR = 0xcccc;  //启动看门狗定时器
}

void watchdog_feed(void)
{
    IWDG->KR=0xaaaa;
}

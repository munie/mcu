// main.c

#include "ctlcenter.h"
#include "watchdog.h"
#include "stm32f10x.h"

static void rcc_config(void)
{
    /* PCLK1 = HCLK/4 */
    RCC_PCLK1Config(RCC_HCLK_Div4);

    /* TIM2 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 | RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);

    /* GPIOC clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
}

static void nvic_config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    // sim
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // gps
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // modbus
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // debug
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void usart1_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 , ENABLE);

    /* Configure USART1 Rx (PA.10) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                  //PA10-RX1 PA3-RX1引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;       //浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* Configure USART1 Tx (PA.09) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;                   //PA9-TX1 PA2-TX2引脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;             //复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 波特率、字长、停止位、奇偶校验位、硬件流控制、异步串口为默认（被屏蔽字设置）
    USART_DeInit(USART1);                                       //串口寄存器初始化
    USART_InitStructure.USART_BaudRate = 115200;                //波特率115200
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长8位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1位停止字节
    USART_InitStructure.USART_Parity = USART_Parity_No;         //无奇偶校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //打开Rx接收和Tx发送功能

    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);                //开串口1接收中断
    USART_Cmd(USART1, ENABLE);                                  //启动串口
}

static void usart2_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);

    /* Configure USART2 Rx (PA.3) as input floating */ 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;                   //PA10-RX1 PA3-RX1引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;       //浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART2 Tx (PA.02) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;                   //PA9-TX1 PA2-TX2引脚
    GPIO_InitStructure.GPIO_Speed =	GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;             //复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 波特率、字长、停止位、奇偶校验位、硬件流控制、异步串口为默认（被屏蔽字设置）
    USART_DeInit(USART2);                                       //串口寄存器初始化
    USART_InitStructure.USART_BaudRate = 19200;                 //波特率19200
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长8位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1位停止字节
    USART_InitStructure.USART_Parity = USART_Parity_No;         //无奇偶校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //打开Rx接收和Tx发送功能

    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);                //开串口2接收中断
    USART_Cmd(USART2, ENABLE);                                  //启动串口2
}

static void usart3_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 , ENABLE);

    /* Configure USART3 Rx (PB.11) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;                  //PB11引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;       //浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* Configure USART3 Tx (PB.10) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10 
    GPIO_InitStructure.GPIO_Speed =	GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;             //复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    // RS485控制脚DE/RE (PB.5)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;                   //PB5引脚(输入输出控制脚)
    GPIO_InitStructure.GPIO_Speed =	GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;            //复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 波特率、字长、停止位、奇偶校验位、硬件流控制、异步串口为默认（被屏蔽字设置）
    USART_DeInit(USART3);                                       //串口寄存器初始化
    USART_InitStructure.USART_BaudRate = 9600;                  //波特率9600
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长8位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1位停止字节
    USART_InitStructure.USART_Parity = USART_Parity_No;         //偶（Even）无奇偶校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//打开Rx接收和Tx发送功能

    USART_Init(USART3, &USART_InitStructure);
    USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);                //开串口3接收中断
    USART_Cmd(USART3, ENABLE);                                  //启动串口3
}

static void usart4_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE); 

    // 配置接收(RX)管脚PC11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    // 配置发送(TX)管脚PC10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    // RS485_2控制脚DE/RE (PC.13)
    USART_DeInit(UART4);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;          // PC13引脚(输入输出控制脚)
    GPIO_InitStructure.GPIO_Speed =	GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // 复用推挽输出
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // 波特率、字长、停止位、奇偶校验位、硬件流控制、异步串口为默认（被屏蔽字设置）
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl =	USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(UART4, &USART_InitStructure);
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_Cmd(UART4, ENABLE);
}

static void tim2_config(void)
{
    // (1 + TIM_Period) / 36M * (1 + TIM_Peroid) = 1s
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Period = 10000 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 3600 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ARRPreloadConfig(TIM2, DISABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

static void tim3_config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Period = 100;
    TIM_TimeBaseStructure.TIM_Prescaler = 7199;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ClearFlag(TIM3, TIM_FLAG_Update);
    TIM_ARRPreloadConfig(TIM3, DISABLE);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

static void gpio_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 开关机控制脚PC6
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // 按键输入(PC2.3.5.7)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

int main(void)
{
    rcc_config();
    nvic_config();
    usart1_config();
//    usart2_config();
    usart3_config();
//    usart4_config();
    tim2_config();
//    tim3_config();
    gpio_config();
    watchdog_init(4, 6250);

    ctlcenter_init(ctlcenter);
    while (1) ctlcenter_perform(ctlcenter, 0);
}

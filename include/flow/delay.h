#ifndef _FLOW_DELAY_H_
#define _FLOW_DELAY_H_

static inline void delay(int next)
{
    for(int i = 0; i < next; i++)
       for(int j = 0; j < 12000; j++) { }
}

#endif

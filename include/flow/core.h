#ifndef _FLOW_CORE_H_
#define _FLOW_CORE_H_

#include "usart.h"
#include "simcard.h"
#include "flowmeter.h"

struct core {
    int count_tim2;
    struct simcard *sim;
    struct flowmeter *meter;
};

extern void core_init(struct core *core);
extern void core_perform(struct core *core);

extern struct core *the_core;

#endif

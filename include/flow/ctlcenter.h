#ifndef _ENVTERM_CTLCENTER_H_
#define _ENVTERM_CTLCENTER_H_

#include "usart.h"
#include "simcard.h"

struct ctlcenter {
    int count_tim2;
    struct simcard *sim;
    struct usart_session *modbus;
};

extern void ctlcenter_init(struct ctlcenter *cnter);
extern void ctlcenter_final(struct ctlcenter *cnter);
extern void ctlcenter_perform(struct ctlcenter *cnter, int next);

extern struct ctlcenter *ctlcenter;

#endif

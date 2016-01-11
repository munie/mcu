#ifndef _FLOW_WATCHDOG_H_
#define _FLOW_WATCHDOG_H_

extern void watchdog_init(unsigned char prer, unsigned int reld);
extern void watchdog_feed(void);

#endif

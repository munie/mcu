#ifndef _ENVTERM_SIM908_H_
#define _ENVTERM_SIM908_H_

#include "usart.h"
#include <time.h>

struct simcard {
    struct usart_session sess;
    struct {
        unsigned char ate : 1;
    } flag;
    char ccid[20+1];
    char csq[2+1];
    char gps_time[15];
    char gps_speed[11];
    char gps_n[10];
    char gps_e[10];
    char voltage[6+1];
    char flow_total[4+1];
};

extern void simcard_init(struct simcard *sim);
extern void simcard_final(struct simcard *sim);
extern void simcard_send_msg_to_center(struct simcard *sim, const char * restrict msg, ...);

extern int simcard_update_ccid(struct simcard *sim);
extern int simcard_update_csq(struct simcard *sim);
extern int simcard_update_gps(struct simcard *sim);

#endif

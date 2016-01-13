#ifndef _FLOW_SIMCARD_H_
#define _FLOW_SIMCARD_H_

#include "usart.h"

#define CCID_LEN 20
#define CSQ_LEN 2
#define GPSN_LEN 9
#define GPSE_LEN 9
#define GPSTIME_LEN 14
#define VOLTAGE_LEN 6

struct simcard {
    struct usart_session sess;
    struct {
        unsigned char ate : 1;
    } flag;
    char ccid[CCID_LEN + 1];
    char csq[CSQ_LEN + 1];
    char gpsn[GPSN_LEN + 1];
    char gpse[GPSE_LEN + 1];
    char gpstime[GPSTIME_LEN + 1];
    char voltage[VOLTAGE_LEN + 1];
};

extern void simcard_init(struct simcard *sim);
extern void simcard_final(struct simcard *sim);
extern void simcard_send_msg_to_center(struct simcard *sim, const char * restrict msg, ...);
extern int simcard_check_network(struct simcard *sim);
extern int simcard_check_gps(struct simcard *sim);
extern int simcard_update_ccid(struct simcard *sim);
extern int simcard_update_csq(struct simcard *sim);
extern int simcard_update_gps(struct simcard *sim);

#endif

#ifndef _FLOW_FLOWMETER_H_
#define _FLOW_FLOWMETER_H_

#include "usart.h"

#define FLOW_RECORD_MAX (24 * 2 * 10)
#define FLOW_TOTAL_LEN 8
#define FLOW_TIME_LEN 19
#define FLOW_HEXCMD_LEN 8

struct flow_record {
    char total[FLOW_TOTAL_LEN + 1];
    char time[FLOW_TIME_LEN + 1];
};

struct flowmeter {
    struct usart_session sess;
    unsigned char hexcmd[FLOW_HEXCMD_LEN];
    struct flow_record *flow_table;
    int flow_size;
    int flow_pos;

    char last_flow_total[FLOW_TOTAL_LEN + 1];
    char last_flow_time[FLOW_TIME_LEN + 1];
};

extern void flowmeter_init(struct flowmeter *meter, struct flow_record *table);
extern void flowmeter_final(struct flowmeter *meter);
extern void flowmeter_fetch_data(struct flowmeter *meter);

#endif

#ifndef _FLOW_FLOWMETER_H_
#define _FLOW_FLOWMETER_H_

#include "usart.h"
#include "list.h"

#define FLOW_TOTAL_LEN 8
#define FLOW_TIME_LEN 19
#define FLOW_HEXCMD_LEN 8

// sizeof (struct flow_record) = 4 + 4 + 9 + 20 = 37
// data size of a day = 37 * 2 * 24 = 1776
// data size of a mon = 1766 * 30 = 53280
// 4K = 4096 & 64K = 65536
// 0x4000 = 4 * 4K = 16384
// 16384 / 1776 = 9.23
#define FLOW_RECORD_MAX (2 * 24 * 9)

struct flow_record {
    struct list_head node;
    char total[FLOW_TOTAL_LEN + 1];
    char time[FLOW_TIME_LEN + 1];
};

struct flowmeter {
    struct usart_session sess;
    unsigned char hexcmd[FLOW_HEXCMD_LEN];
    struct flow_record *flow_table;
};

extern void flowmeter_init(struct flowmeter *meter);
extern void flowmeter_final(struct flowmeter *meter);
extern void flowmeter_fetch_data(struct flowmeter *meter);

#endif

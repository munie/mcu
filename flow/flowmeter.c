// flowmeter.c

#include "flowmeter.h"
#include "util.h"
#include "core.h"
#include <stdio.h>
#include <stdlib.h>

//#define SH_FANYANG
#define WX_XUNDA
unsigned char const SH_FANYANG_CMD[FLOW_HEXCMD_LEN] = {0x01, 0x03, 0x00, 0x08, 0x00, 0x02, 0x45, 0xC9};
unsigned char const WX_XUNDA_CMD[FLOW_HEXCMD_LEN] = {0x01, 0x04, 0x00, 0x05, 0x00, 0x05, 0x20, 0x08};

// flow meter ===========================================================

static void flowmeter_usart_parse(struct usart_session *sess)
{
    struct flowmeter *meter = (struct flowmeter *)((char *)sess - (size_t)(&((struct flowmeter *)0)->sess));

    if (list_size(&meter->flow_table->node) >= FLOW_RECORD_MAX) return;

    #if defined SH_FANYANG
    if(*RFIFOP(sess, 0) == 0x01 && *RFIFOP(sess, 1) == 0x03 && *RFIFOP(sess, 2) == 0x0A) {
    #elif defined WX_XUNDA
    if(*RFIFOP(sess, 0) == 0x01 && *RFIFOP(sess, 1) == 0x04 && *RFIFOP(sess, 2) == 0x0A) {
    #endif
        // update current/last flow total
        sprintf(meter->flow_table->total, "%.2X%.2X%.2X%.2X", *RFIFOP(sess, 3),
            *RFIFOP(sess, 4), *RFIFOP(sess, 5), *RFIFOP(sess, 6));

        // update current/last flow time
        if (strlen(the_core->sim->gpstime) != 0) {
            struct tm tm = {0};
            time_parse_gpstime(&tm, the_core->sim->gpstime, GPSTIME_LEN);
            time_add_hours(&tm, 8);
            time_to_string_cn(&tm, meter->flow_table->time, FLOW_TIME_LEN);
        }

        // store current/last record to flow record table
        struct flow_record *record = (struct flow_record *)malloc(sizeof(struct flow_record));
        list_head_init(&record->node);
        memcpy(record->total, meter->flow_table->total, FLOW_TOTAL_LEN);
        memcpy(record->time, meter->flow_table->time, FLOW_TIME_LEN);
        list_add_tail(&record->node, &meter->flow_table->node);
    }
    usart_rfifo_skip_all(sess);
}

void flowmeter_init(struct flowmeter *meter)
{
    usart_init(&meter->sess, USART3, GPIOB, GPIO_Pin_5, flowmeter_usart_parse);
    usart_add(&meter->sess);

    #if defined SH_FANYANG
    memcpy(meter->hexcmd, SH_FANYANG_CMD, FLOW_HEXCMD_LEN);
    #elif defined WX_XUNDA
    memcpy(meter->hexcmd, WX_XUNDA_CMD, FLOW_HEXCMD_LEN);
    #endif
    meter->flow_table = (struct flow_record*)malloc(sizeof(struct flow_record));
    list_head_init(&meter->flow_table->node);
}

void flowmeter_final(struct flowmeter *meter)
{
    usart_del(&meter->sess);
}

void flowmeter_fetch_data(struct flowmeter *meter)
{
    memcpy(meter->sess.wdata, meter->hexcmd, FLOW_HEXCMD_LEN);
    WFIFOSET((&meter->sess), FLOW_HEXCMD_LEN);
}

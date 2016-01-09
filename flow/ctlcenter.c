// ctlcenter.c

#include "ctlcenter.h"
#include "delay.h"
#include <stdio.h>

unsigned char const CGQ_CMD[8]={0x01, 0x03, 0x00, 0x08, 0x00, 0x02, 0x45, 0xC9};

// flow record table ================================================================

struct flow_record {
    char total[4+1];
    char time[15];
};

#define FLOW_RECORD_MAX 24 * 10
static struct flow_record flow_table[FLOW_RECORD_MAX];
static int flow_size;
static int flow_pos;

// ctlcenter ===================================================================

static struct ctlcenter _ctlcenter;
struct ctlcenter *ctlcenter = &_ctlcenter;
static struct simcard _simcard;
static struct usart_session _flowmeter;

static void flowmeter_usart_parse(struct usart_session *sess)
{
    if(*RFIFOP(sess, 0) == 0x01 && *RFIFOP(sess, 1) == 0x03
        && *RFIFOP(sess, 2) == 0x04 && *(unsigned char*)RFIFOP(sess, 9) == 0xff) {
        // update current flow_total & flow_time in simcard
        memcpy(_simcard.current_flow_total, RFIFOP(sess, 3), 4);
        memcpy(_simcard.current_flow_time, _simcard.gps_time, 15);

        // store record to record table
        if (flow_size - flow_pos != FLOW_RECORD_MAX && flow_size - flow_pos != -1) {
            memcpy(flow_table[flow_size].total, _simcard.current_flow_total, 4);
            memcpy(flow_table[flow_size].time, _simcard.current_flow_time, 15);
            if (++flow_size == FLOW_RECORD_MAX) flow_size = 0;
        }
    }
    usart_rfifo_skip(sess, RFIFOREST(sess));
}

void ctlcenter_init(struct ctlcenter *cnter)
{
    cnter->sim = &_simcard;
    simcard_init(cnter->sim);

    cnter->flowmeter = &_flowmeter;
    usart_init(cnter->flowmeter, USART3, flowmeter_usart_parse);
    usart_add(cnter->flowmeter);
}

void ctlcenter_final(struct ctlcenter *cnter) { }

void ctlcenter_perform(struct ctlcenter *cnter, int next)
{
    usart_perform(0);

    // 2 mins : update csq & gps
    if (cnter->count_tim2 % 120 == 45) {
        simcard_update_csq(cnter->sim);
        simcard_update_gps(cnter->sim);

    // 10 mins : send alive packet
    } else if (cnter->count_tim2 % 600 == 60)
        simcard_send_msg_to_center(cnter->sim, "/flow/alive?ccid=%s\r\n", cnter->sim->ccid);

    // 30 mins : send command to flowmeter to get flow data
    else if (cnter->count_tim2 % 1800 == 120)
        memcpy(cnter->flowmeter->wdata, CGQ_CMD, 8);

    // 30 mins : send flow record
    else if (cnter->count_tim2 % 1800 == 180) {
        while (flow_size != flow_pos) {
            // send flow record
            simcard_send_msg_to_center(cnter->sim, "/flow/record?ccid=%s&flow_total=%s&time=%s\r\n",
                    cnter->sim->ccid, flow_table[flow_pos].total, flow_table[flow_pos].time);

            // return if not received *OK#
            delay(10000);
            if (strstr(RFIFOP((&cnter->sim->sess), 0), "*OK#") == NULL)
                break;

            // update record_pos
            if (++flow_pos == FLOW_RECORD_MAX) flow_pos = 0;
        }

    // 60 mins : reset
    } else if (cnter->count_tim2 >= 3600)
        cnter->count_tim2 = 0;
}

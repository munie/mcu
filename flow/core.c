// core.c

#include "core.h"
#include "delay.h"
#include <stdio.h>

unsigned char const CGQ_CMD[8]={0x01, 0x03, 0x00, 0x08, 0x00, 0x02, 0x45, 0xC9};

// flow record table ===========================================================

struct flow_record {
    char total[8+1];
    char time[14+1];
};

#define FLOW_RECORD_MAX (24 * 2 * 5)
static struct flow_record flow_table[FLOW_RECORD_MAX];
static int flow_size;
static int flow_pos;

// core ===================================================================

static struct core _core;
struct core *core = &_core;
static struct simcard _simcard;
static struct usart_session _flowmeter;

static void flowmeter_usart_parse(struct usart_session *sess)
{
    if(*RFIFOP(sess, 0) == 0x01 && *RFIFOP(sess, 1) == 0x03
        && *RFIFOP(sess, 2) == 0x04 && *(unsigned char*)RFIFOP(sess, 9) == 0xff) {
        // update current flow_total & flow_time in simcard
        sprintf(_simcard.current_flow_total, "%X%X%X%X", *RFIFOP(sess, 3),
            *RFIFOP(sess, 4), *RFIFOP(sess, 5), *RFIFOP(sess, 6));
        memcpy(_simcard.current_flow_time, _simcard.gps_time, 14);

        // store record to flow record table
        if (flow_size - flow_pos != FLOW_RECORD_MAX && flow_size - flow_pos != -1) {
            memcpy(flow_table[flow_size].total, _simcard.current_flow_total, 4);
            memcpy(flow_table[flow_size].time, _simcard.current_flow_time, 14);
            if (++flow_size == FLOW_RECORD_MAX) flow_size = 0;
        }
    }
    usart_rfifo_skip(sess, RFIFOREST(sess));
}

void core_init(struct core *core)
{
    core->sim = &_simcard;
    simcard_init(core->sim);

    core->flowmeter = &_flowmeter;
    usart_init(core->flowmeter, USART3, NULL, 0, flowmeter_usart_parse);
    usart_add(core->flowmeter);
}

void core_perform(struct core *core)
{
    usart_perform();

    // 2 mins : update csq & gps
    if (core->count_tim2 % 120 == 45) {
        simcard_update_csq(core->sim);
        simcard_update_gps(core->sim);

    // 10 mins : send alive packet
    } else if (core->count_tim2 % 600 == 60)
        simcard_send_msg_to_center(core->sim, "/flow/alive?ccid=%s\r\n", core->sim->ccid);

    // 30 mins : send command to flowmeter to get flow data
    else if (core->count_tim2 % 1800 == 120) {
        memcpy(core->flowmeter->wdata, CGQ_CMD, 8);
        WFIFOSET(core->flowmeter, 8);
        delay(1000);

    // 30 mins : send flow record
    } else if (core->count_tim2 % 1800 == 180) {
        // CAUTION! maybe loop endlessly
        while (flow_size != flow_pos) {
            // send flow record
            simcard_send_msg_to_center(core->sim, "/flow/record?ccid=%s&flow_total=%s&time=%s\r\n",
                core->sim->ccid, flow_table[flow_pos].total, flow_table[flow_pos].time);

            // return if not received *OK# in 10 sec
            delay(10000);
            if (strstr(RFIFOP((&core->sim->sess), 0), "*OK#") == NULL)
                break;

            // update flow_pos
            if (++flow_pos == FLOW_RECORD_MAX) flow_pos = 0;
        }

    // 60 mins : reset
    } else if (core->count_tim2 >= 3600)
        core->count_tim2 = 0;
}

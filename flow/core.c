// core.c

#include "core.h"
#include "delay.h"
#include "util.h"
#include <stdio.h>

unsigned char const CGQ_CMD[8]={0x01, 0x03, 0x00, 0x08, 0x00, 0x02, 0x45, 0xC9};

// flow record table ===========================================================

struct flow_record {
    char total[CURRENT_FLOW_TOTAL_LEN + 1];
    char time[CURRENT_FLOW_TIME_LEN + 1];
};

#define FLOW_RECORD_MAX (24 * 2 * 10)
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
        // update current flow total
        sprintf(_simcard.current_flow_total, "%.2X%.2X%.2X%.2X", *RFIFOP(sess, 3),
            *RFIFOP(sess, 4), *RFIFOP(sess, 5), *RFIFOP(sess, 6));

        // update current flow time
        if (strlen(_simcard.gpstime) != 0) {
            struct tm tm = {0};
            time_parse_gpstime(&tm, _simcard.gpstime, GPSTIME_LEN);
            time_add_hours(&tm, 8);
            time_to_string_cn(&tm, _simcard.current_flow_time, CURRENT_FLOW_TIME_LEN);
        }

        // store record to flow record table
        if (flow_size - flow_pos != FLOW_RECORD_MAX && flow_size - flow_pos != -1) {
            memcpy(flow_table[flow_size].total, _simcard.current_flow_total, CURRENT_FLOW_TOTAL_LEN);
            memcpy(flow_table[flow_size].time, _simcard.current_flow_time, CURRENT_FLOW_TOTAL_LEN);
            if (++flow_size == FLOW_RECORD_MAX) flow_size = 0;
        }
    }
    usart_rfifo_skip(sess, RFIFOREST(sess));
}

static void check_timer(struct core *core)
{
    // 2 mins : update csq & gps
    if (core->count_tim2 % 120 == 45) {
        simcard_update_csq(core->sim);
        simcard_update_gps(core->sim);

    // 10 mins : send alive packet
    } else if (core->count_tim2 % 600 == 60) {
        simcard_send_msg_to_center(core->sim, "/flow/alive?ccid=%s\r\n", core->sim->ccid);
        delay(1000);

    // 30 mins : send command to flowmeter to get flow data
    } else if (core->count_tim2 % 1800 == 120) {
        memcpy(core->flowmeter->wdata, CGQ_CMD, sizeof CGQ_CMD);
        WFIFOSET(core->flowmeter, sizeof CGQ_CMD);
        delay(1000);

    // 30 mins : send flow record
    } else if (core->count_tim2 % 1800 == 180) {
        // CAUTION! maybe loop endlessly
        while (flow_size != flow_pos) {
            // send flow record
            simcard_send_msg_to_center(core->sim, "/flow/record?ccid=%s&csq=%s&voltage=%s&gpsn=%s&gpse=%s&flow_total=%s&time=%s&pos=%d-%d&response=*OK#\r\n",
                core->sim->ccid, core->sim->csq, core->sim->voltage, core->sim->gpsn, core->sim->gpse,
                flow_table[flow_pos].total, flow_table[flow_pos].time, flow_pos, flow_size);

            // return if not received *OK# in 10 sec
            delay(10000);
            char *tmp = strstr(RFIFOP((&core->sim->sess), 0), "*OK#");
            if (tmp == NULL || tmp > RFIFOP((&core->sim->sess), RFIFOREST((&core->sim->sess))))
                break;

            // update flow_pos
            if (++flow_pos == FLOW_RECORD_MAX) flow_pos = 0;
        }

    // 60 * 24 mins : reset
    } else if (core->count_tim2 >= 3600 * 24)
        core->count_tim2 = 0;
}

void core_init(struct core *core)
{
    core->sim = &_simcard;
    simcard_init(core->sim);

    core->flowmeter = &_flowmeter;
    usart_init(core->flowmeter, USART3, GPIOB, GPIO_Pin_5, flowmeter_usart_parse);
    usart_add(core->flowmeter);
}

void core_perform(struct core *core)
{
    usart_perform();
    check_timer(core);
}

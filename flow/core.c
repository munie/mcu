// core.c

#include "core.h"
#include "delay.h"
#include <stdlib.h>

static struct core __core;
struct core *the_core = &__core;
static struct simcard __simcard;
static struct flowmeter __flowmeter;

// core ===================================================================

static inline void timer_check_simcard(int count, struct simcard *sim)
{
    // 2 mins : update csq & gps
    if (count % 120 == 45) {
        simcard_check_network(sim);
        simcard_check_gps(sim);
        simcard_update_csq(sim);
        simcard_update_gps(sim);
        delay(1000);
    }
}

static inline void timer_send_alive_packet(int count, struct simcard *sim)
{
    // 5 mins : send alive packet
    if (count % 300 == 60) {
        simcard_send_msg_to_center(sim, "/flow/alive?ccid=%s\r\n", sim->ccid);
        delay(1000);
    }
}

static inline void timer_fetch_flow_data(int count, struct flowmeter *meter)
{
    // 30 mins : send command to flowmeter to get flow data
    if (count % 1800 == 120) {
        flowmeter_fetch_data(meter);
        delay(1000);
    }
}

static inline void timer_send_flow_record(int count, struct simcard *sim, struct flowmeter *meter)
{
    // 30 mins : send flow record
    if (count % 1800 == 180) {
        // CAUTION! maybe loop endlessly
        while (!list_empty(&meter->flow_table->node)) {
            struct flow_record *record = list_entry(meter->flow_table->node.next, struct flow_record, node);
            // send flow record
            simcard_send_msg_to_center(sim, "/flow/record?ccid=%s&csq=%s&voltage=%s&gpsn=%s&gpse=%s&flow_total=%s&time=%s&size=%d&response=*OK#\r\n",
                sim->ccid, sim->csq, sim->voltage, sim->gpsn, sim->gpse,
                record->total, record->time, list_size(&meter->flow_table->node));

            // return if not received *OK# in 10 sec
            delay(10000);
            char *tmp = strstr(RFIFOP((&sim->sess), 0), "*OK#");
            if (tmp == NULL || tmp > RFIFOP((&sim->sess), RFIFOREST((&sim->sess))))
                break;
            usart_rfifo_skip(&sim->sess, RFIFOREST((&sim->sess)));

            // delete record
            list_del(&record->node);
            free(record);
        }
    }
}

void core_init(struct core *core)
{
    // sim card init
    core->sim = &__simcard;
    simcard_init(core->sim);

    // flow meter init
    core->meter = &__flowmeter;
    flowmeter_init(core->meter);
}

void core_exec(struct core *core)
{
    usart_exec();

    timer_check_simcard(core->count_tim2, core->sim);
    timer_send_alive_packet(core->count_tim2, core->sim);
    timer_fetch_flow_data(core->count_tim2, core->meter);
    timer_send_flow_record(core->count_tim2, core->sim, core->meter);

    if (core->count_tim2 > 3600 * 24) core->count_tim2 = 0;
}

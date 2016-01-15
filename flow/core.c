// core.c

#include "core.h"
#include "delay.h"

static struct core __core;
struct core *the_core = &__core;
static struct simcard __simcard;
static struct flowmeter __flowmeter;
static struct flow_record flow_table[FLOW_RECORD_MAX];

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
    // 10 mins : send alive packet
    if (count % 600 == 60) {
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
        while (meter->flow_size != meter->flow_pos) {
            // send flow record
            simcard_send_msg_to_center(sim, "/flow/record?ccid=%s&csq=%s&voltage=%s&gpsn=%s&gpse=%s&flow_total=%s&time=%s&pos=%d-%d&response=*OK#\r\n",
                sim->ccid, sim->csq, sim->voltage, sim->gpsn, sim->gpse,
                meter->flow_table[meter->flow_pos].total,
                meter->flow_table[meter->flow_pos].time,
                meter->flow_pos, meter->flow_size);

            // return if not received *OK# in 10 sec
            delay(10000);
            char *tmp = strstr(RFIFOP((&sim->sess), 0), "*OK#");
            if (tmp == NULL || tmp > RFIFOP((&sim->sess), RFIFOREST((&sim->sess))))
                break;

            // update flow_pos
            if (++meter->flow_pos == FLOW_RECORD_MAX) meter->flow_pos = 0;
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
    flowmeter_init(core->meter, flow_table);
}

void core_perform(struct core *core)
{
    usart_perform();

    timer_check_simcard(core->count_tim2, core->sim);
    timer_send_alive_packet(core->count_tim2, core->sim);
    timer_fetch_flow_data(core->count_tim2, core->meter);
    timer_send_flow_record(core->count_tim2, core->sim, core->meter);

    if (core->count_tim2 > 3600 * 24) core->count_tim2 = 0;
}

// core.c

#include "core.h"
#include "delay.h"

// core ===================================================================

static struct core _core;
struct core *core = &_core;
static struct simcard _simcard;
static struct flowmeter _flowmeter;
static struct flow_record flow_table[FLOW_RECORD_MAX];

static void check_timer(struct core *core)
{
    struct simcard *sim = core->sim;
    struct flowmeter *meter = core->meter;
    
    // 2 mins : update csq & gps
    if (core->count_tim2 % 120 == 45) {
        simcard_update_csq(sim);
        simcard_update_gps(sim);
        delay(1000);

    // 10 mins : send alive packet
    } else if (core->count_tim2 % 600 == 60) {
        simcard_send_msg_to_center(sim, "/flow/alive?ccid=%s\r\n", sim->ccid);
        delay(1000);

    // 30 mins : send command to flowmeter to get flow data
    } else if (core->count_tim2 % 1800 == 120) {
        flowmeter_fetch_data(meter);
        delay(1000);

    // 30 mins : send flow record
    } else if (core->count_tim2 % 1800 == 180) {
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

    // 60 * 24 mins : reset
    } else if (core->count_tim2 >= 3600 * 24)
        core->count_tim2 = 0;
}

void core_init(struct core *core)
{
    // sim card init
    core->sim = &_simcard;
    simcard_init(core->sim);

    // flow meter init
    core->meter = &_flowmeter;
    flowmeter_init(core->meter, flow_table);
}

void core_perform(struct core *core)
{
    usart_perform();
    check_timer(core);
}

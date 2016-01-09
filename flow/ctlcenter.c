// ctlcenter.c

#include "ctlcenter.h"
#include "delay.h"
#include <stdio.h>

unsigned char const CGQ_CMD[8]={0x01, 0x03, 0x00, 0x08, 0x00, 0x02, 0x45, 0xC9};

static struct ctlcenter _ctlcenter;
struct ctlcenter *ctlcenter = &_ctlcenter;
static struct simcard _simcard;

// ctlcenter ===================================================================

static struct usart_session _flowmeter;
static void flowmeter_usart_parse(struct usart_session *sess)
{
    if(*RFIFOP(sess, 0) == 0x01 && *RFIFOP(sess, 1) == 0x03
        && *RFIFOP(sess, 2) == 0x04 && *(unsigned char*)RFIFOP(sess, 9) == 0xff) {
        memcpy(_simcard.flow_total, RFIFOP(sess, 3), 4);
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

    if (cnter->count_tim2 % 120 == 45) {
        simcard_update_csq(cnter->sim);
        simcard_update_gps(cnter->sim);
    } else if (cnter->count_tim2 % 600 == 60) {
        simcard_send_msg_to_center(cnter->sim, "/flow/alive?ccid=%s\r\n", cnter->sim->ccid);
        delay(1000);
    } else if (cnter->count_tim2 % 1800 == 120) {
        memcpy(cnter->flowmeter->wdata, CGQ_CMD, 8);
    } else if (cnter->count_tim2 >= 3600)
        cnter->count_tim2 = 0;
}

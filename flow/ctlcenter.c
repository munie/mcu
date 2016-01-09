// ctlcenter.c

#include "ctlcenter.h"
#include "delay.h"
#include <stdio.h>

static struct ctlcenter _ctlcenter;
struct ctlcenter *ctlcenter = &_ctlcenter;
static struct simcard _simcard;

// ctlcenter ===================================================================

static struct usart_session _modbus;
static void modbus_usart_parse(struct usart_session *sess)
{
    if(*RFIFOP(sess, 0) == 0x01 && *RFIFOP(sess, 1) == 0x03
        && *RFIFOP(sess, 2) == 0x04 && *(unsigned char*)RFIFOP(sess, 9) == 0xff) {
        memcpy(_simcard.flow_total, RFIFOP(sess, 3), 4);
        simcard_send_msg_to_center(&_simcard, "/flow/record?ccid=%s&csq=%s&voltage=%s&flow_total=%s\r\n",
            _simcard.ccid, _simcard.csq, _simcard.voltage, _simcard.flow_total);
    }
    usart_rfifo_skip(sess, RFIFOREST(sess));
}

void ctlcenter_init(struct ctlcenter *cnter)
{
    cnter->sim = &_simcard;
    simcard_init(cnter->sim);

    cnter->modbus = &_modbus;
    usart_init(cnter->modbus, USART3, modbus_usart_parse);
    usart_add(cnter->modbus);
}

void ctlcenter_final(struct ctlcenter *cnter) { }

void ctlcenter_perform(struct ctlcenter *cnter, int next)
{
    usart_perform(0);

    if (ctlcenter->count_tim2 % 120 == 45) {
        simcard_update_csq(cnter->sim);
        simcard_update_gps(cnter->sim);
    } else if (ctlcenter->count_tim2 % 600 == 60) {
        simcard_send_msg_to_center(cnter->sim, "/flow/alive?ccid=%s\r\n", cnter->sim->ccid);
        delay(1000);
    }
    else if (ctlcenter->count_tim2 >= 3600)
        ctlcenter->count_tim2 = 0;
}

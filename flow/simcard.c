// simcard.c

#include "simcard.h"
#include "delay.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include "core.h"

// init commands of AT
#define AT_TEST         "AT\n"
#define AT_ECHO_CLOSE   "ATE0\n"
#define AT_POWER_DOWN   "AT+CPOWD=1\n"
#define AT_GPS_PWR      "AT+CGNSPWR=1\n"

// query commands of AT
#define AT_QUERY_CCID   "AT+CCID\n"
#define AT_QUERY_CSQ    "AT+CSQ\n"
#define AT_QUERY_GPS    "AT+CGNSINF\n"

// tcp commands of AT
#define AT_CG_CLASS     "AT+CGCLASS=\"B\"\n"
#define AT_CG_DCONT     "AT+CGDCONT=1,\"IP\",\"CMNET\"\n"
#define AT_CG_ATT       "AT+CGATT=1\n"
#define AT_CIP_CSGP     "AT+CIPCSGP=1,\"CMNET\"\n"
#define AT_CIP_START    "AT+CIPSTART=\"TCP\",\"zjhtht.cn\",\"3002\"\n"
//#define AT_CIP_START    "AT+CIPSTART=\"TCP\",\"munie.ddns.net\",\"5964\"\n"
#define AT_CIP_SEND     "AT+CIPSEND\n"

#define LENGTH_AT(cmd) (strlen(cmd) - 1)

static bool send_command_synchronized(struct usart_session *sess,
    const char * restrict cmd, const char * restrict res, int res_offset)
{
    usart_rfifo_skip_all(sess);
    usart_sendstr_session(sess, cmd);
    delay(1000);

    // compare AT command
    if (memcmp(RFIFOP(sess, 0), cmd, LENGTH_AT(cmd)) != 0)
        goto out_error;
    usart_rfifo_skip_windows_line(sess);

    // compare result
    if (memcmp(RFIFOP(sess, 0), res, strlen(res)) != 0)
        goto out_error;
    usart_rfifo_skip_windows_line(sess);

    return true;
    out_error:
    usart_rfifo_skip_all(sess);
    return false;
}

static void send_command(struct usart_session *sess, const char * restrict cmd, int next)
{
    usart_sendstr_session(sess, cmd);
    delay(next);
}

static void poweron()
{
    GPIO_SetBits(GPIOC,GPIO_Pin_6);
    delay(600);
    GPIO_ResetBits(GPIOC,GPIO_Pin_6);
    delay(600);
    delay(1500);
}

static void simcard_connect(struct simcard *sim)
{
    struct usart_session *sess = &sim->sess;
    send_command(sess, AT_CG_CLASS, 1000);
    send_command(sess, AT_CG_DCONT, 1000);
    send_command(sess, AT_CG_ATT, 1000);
    send_command(sess, AT_CIP_CSGP, 1000);
    send_command(sess, AT_CIP_START, 5000);
    usart_rfifo_skip_all(sess);
}

// simcard ======================================================================

static void simcard_parse_AT(struct simcard *sim)
{
    struct usart_session *sess = &sim->sess;

    // update ccid
    if (memcmp(RFIFOP(sess, 0), AT_QUERY_CCID, LENGTH_AT(AT_QUERY_CCID)) == 0) {
        // query ccid, retval: "\r\n8986...\r\n"
        usart_rfifo_skip_windows_line(sess);
        memcpy(sim->ccid, RFIFOP(sess, 0), CCID_LEN);
        usart_rfifo_skip_windows_line(sess);

    // update csq
    } else if (memcmp(RFIFOP(sess, 0), AT_QUERY_CSQ, LENGTH_AT(AT_QUERY_CSQ)) == 0) {
        // query csq, retval: "\r\n+CSQ: 19,0\r\n"
        usart_rfifo_skip_windows_line(sess);
        memcpy(sim->csq, RFIFOP(sess, 6), CSQ_LEN);
        usart_rfifo_skip_windows_line(sess);

    // update gps
    } else if (memcmp(RFIFOP(sess, 0), AT_QUERY_GPS, LENGTH_AT(AT_QUERY_GPS)) == 0) {
        // query gps, retval: "\r\n+CGNSINF: 1,1,20160107070129.000,28.652770,121.446185,5.200,0.06,231.5,1,,3.8,5.7,4.2,,6,5,,,51,,\r\n"
        // fail when received "+CGNSINF: 1,0,19800106000349.000,,,,0.00,0.0,0,,,,,,0,0,,,,,"
        usart_rfifo_skip_windows_line(sess);
        if (*RFIFOP(sess, 2 + 12) == '1') {
            memcpy(sim->gpstime, RFIFOP(sess, 14), GPSTIME_LEN);
            memcpy(sim->gpsn, RFIFOP(sess, 33), GPSN_LEN);
            memcpy(sim->gpse, RFIFOP(sess, 43), GPSE_LEN);
        }
        usart_rfifo_skip_windows_line(sess);

    // unknown
    } else {
        usart_rfifo_skip_windows_line(sess);
        usart_rfifo_skip_windows_line(sess);
    }
}

static void simcard_parse_packet(struct simcard *sim)
{
    struct usart_session *sess = &sim->sess;

    // response data
    if (memcmp(RFIFOP(sess, 0), "!A1?", 4) == 0) {
        simcard_send_msg_to_center(sim, "/flow/record?ccid=%s&csq=%s&voltage=%s&gpsn=%s&gpse=%s&flow_total=%s&time=%s\r\n",
            sim->ccid, sim->csq, sim->voltage, sim->gpsn, sim->gpse,
            the_core->meter->flow_table->total, the_core->meter->flow_table->time);
        usart_rfifo_skip_windows_line(sess);

    // response state
    } else if (memcmp(RFIFOP(sess, 0), "!A3?", 4) == 0) {
        simcard_send_msg_to_center(sim, "/flow/state?ccid=%s&csq=%s&voltage=%s&gpsn=%s&gpse=%s&gpstime=%s\r\n",
            sim->ccid, sim->csq, sim->voltage, sim->gpsn, sim->gpse, sim->gpstime);
        usart_rfifo_skip_windows_line(sess);

    // unknown
    } else
        usart_rfifo_skip_windows_line(sess);
}

static void simcard_usart_parse(struct usart_session *sess)
{
    struct simcard *sim = (struct simcard *)((char *)sess - (size_t)(&((struct simcard *)0)->sess));

    if (memcmp(RFIFOP(sess, 0), "AT", 2) == 0)
        simcard_parse_AT(sim);
    else
        simcard_parse_packet(sim);
}

void simcard_init(struct simcard *sim)
{
    usart_init(&sim->sess, USART1, NULL, 0, simcard_usart_parse);
    usart_add(&sim->sess);
    struct usart_session *sess = &sim->sess;

    redo:
    if (send_command_synchronized(sess, AT_TEST, "OK", 2))
        goto out;

    send_command_synchronized(sess, AT_POWER_DOWN, "OK", 2);
    delay(10 * 1000);
    poweron();
    goto redo;

    out:
    send_command(sess, AT_GPS_PWR, 1000);
    send_command(sess, AT_QUERY_CCID, 1000);
    return;
}

void simcard_final(struct simcard *sim)
{
    usart_del(&sim->sess);
}

void simcard_send_msg_to_center(struct simcard *sim, const char * restrict msg, ...)
{
    struct usart_session *sess = &sim->sess;

    va_list arg;
    va_start(arg, msg);
    vsprintf(sess->wdata, msg, arg);
    va_end(arg);

//    if (simcard_check_network(sim) == 0) {
//        send_command(sess, AT_CIP_SEND, 0);
//        send_command(sess, sess->wdata, 0);
//        send_command(sess, "\x1a\r\n", 0);
//    }

    redo:
    send_command(sess, AT_CIP_SEND, 500);
    if (memcmp(RFIFOP(sess, RFIFOREST(sess)) - 9, "\r\nERROR\r\n", 9) == 0) {
        simcard_connect(sim);
        goto redo;
    }
    send_command(sess, sess->wdata, 0);
    send_command(sess, "\x1a\r\n", 0);
}

int simcard_check_network(struct simcard *sim)
{
    return 0;
}

int simcard_check_gps(struct simcard *sim)
{
    return 0;
}

void simcard_update_csq(struct simcard *sim)
{
    send_command(&sim->sess, AT_QUERY_CSQ, 1000);
}

void simcard_update_gps(struct simcard *sim)
{
    send_command(&sim->sess, AT_QUERY_GPS, 1000);
}

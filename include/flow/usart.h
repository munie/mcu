#ifndef _FLOW_USART_H_
#define _FLOW_USART_H_

#include "stm32f10x.h"
#include <stddef.h>
#include <string.h>

#define RDATA_MAX 256
#define WDATA_MAX 256
#define USART_SESSION_MAX 4

#define RFIFOP(sess, offset) (sess->rdata + sess->rdata_pos + (offset))
#define RFIFOREST(sess) (sess->rdata_size - sess->rdata_pos)
#define RFIFOSPACE(sess) (RDATA_MAX - sess->rdata_size)
#define WFIFOSET(sess, len) (sess->wdata_size += len)

struct usart_session;
typedef void (*usart_parse_t)(struct usart_session *sess);

struct usart_session {
    USART_TypeDef *usart;
    GPIO_TypeDef *ctrl_io;
    uint16_t ctrl_pin;

    char rdata[RDATA_MAX];
    size_t rdata_size, rdata_pos;
    char wdata[WDATA_MAX];
    size_t wdata_size;

    usart_parse_t usart_parse;
};

extern void usart_rfifo_skip(struct usart_session *sess, size_t len);
extern void usart_rfifo_flush(struct usart_session *sess);
extern void usart_sendstr_session(struct usart_session *sess, const char * restrict data);
extern void usart_send_session(struct usart_session *sess, const unsigned char * restrict data, size_t len);

extern void usart_init(struct usart_session *sess, USART_TypeDef *usart,
    GPIO_TypeDef *io, uint16_t pin, usart_parse_t parse);
extern int usart_add(struct usart_session *sess);
extern void usart_del(struct usart_session *sess);
extern void usart_perform(void);

#endif

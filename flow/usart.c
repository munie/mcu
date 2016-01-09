// usart.c

#include "usart.h"
#include "delay.h"

// usart_session ===============================================================

static void sendchar(USART_TypeDef* usart, const char c)
{
    while (!(usart->SR & USART_FLAG_TXE)) { }
    usart->DR = c;
    usart->SR &= ~USART_FLAG_TXE;
}

static void usart_send(struct usart_session *sess)
{
    for (int i = 0; i < sess->wdata_size; i++)
        sendchar(sess->usart, sess->wdata[i]);
    sess->wdata_size = 0;
}

void usart_rfifo_skip(struct usart_session *sess, size_t len)
{
    if (sess->rdata_pos + len > sess->rdata_size)
        sess->rdata_pos = sess->rdata_size;
    else
        sess->rdata_pos += len;
}

void usart_rfifo_flush(struct usart_session *sess)
{
    if (sess->rdata_size == sess->rdata_pos) {
        sess->rdata_size = sess->rdata_pos = 0;
    } else {
        sess->rdata_size -= sess->rdata_pos;
        memmove(sess->rdata, sess->rdata+sess->rdata_pos, sess->rdata_size);
        sess->rdata_pos = 0;
    }
}

void usart_sendstr_session(struct usart_session *sess, const char * restrict data)
{
    for (int i = 0; i < strlen(data); i++)
        sendchar(sess->usart, data[i]);
}

// usart controller ============================================================

static struct usart_session *usart_table[USART_SESSION_MAX];

void usart_init(struct usart_session *sess, USART_TypeDef *usart, usart_parse_t parse)
{
    sess->usart = usart;
    sess->rdata_size = 0;
    sess->rdata_pos = 0;
    sess->wdata_size = 0;
    sess->usart_parse = parse;
}

int usart_add(struct usart_session *sess)
{
    for (int i = 0; i < USART_SESSION_MAX; i++) {
        if (usart_table[i] == NULL) {
            usart_table[i] = sess;
            return 0;
        }
    }
    
    return -1;
}

void usart_del(struct usart_session *sess)
{
    for (int i = 0; i < USART_SESSION_MAX; i++) {
        if (usart_table[i] == sess) {
            usart_table[i] = NULL;
            break;
        }
    }
}

void usart_perform(int next)
{
    for (int i = 0; i < USART_SESSION_MAX; i++) {
        if (usart_table[i] == NULL)
            continue;
        
        if (usart_table[i]->rdata_size != 0) {
            delay(200);
            if (usart_table[i]->usart_parse != NULL)
                usart_table[i]->usart_parse(usart_table[i]);
            else
                usart_table[i]->rdata_size = 0;
        }

        if (usart_table[i]->wdata_size != 0) {
            usart_send(usart_table[i]);
            usart_table[i]->wdata_size = 0;
        }
    }
}

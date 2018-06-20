#ifndef WS_H
#define WS_H

#include <libwebsockets.h>

#define MAXLEN 20000

int callbackOgar(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len); 

int sendCommand(struct lws *wsi, unsigned char *buf, unsigned int len);

int forceExit;

typedef struct s_packet 
{
    unsigned char buf[MAXLEN+LWS_PRE];
    unsigned int len;
    struct s_packet *next;
} t_packet;

#endif
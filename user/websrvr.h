#ifndef WEBSRVR_H
#define WEBSRVR_H

#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "gpio.h"
#include "tcp_streamer.h"
#include "strbuf.h"

typedef struct _param
{
    strBuf name;
    unsigned char value;
} Param;


extern tcp_streamer* streamsOut;
extern tcp_streamer* streamsInp;

static const unsigned short m1=(unsigned short int)(-1);

void answer500(struct espconn * conn, const size_t code);

bool unpackParams(const strBuf* params, Param* ret, const unsigned short int count);

unsigned short int findParam(const strBuf* name, const Param* params, const unsigned short int count);

bool doReply(const strBuf* inputMessageAll,struct espconn *conn);

void sendStatus(char status, strBuf *send);

void makeHTTPReply(const strBuf* statusString, const size_t contentSize, const strBuf *etag, strBuf *out);

#endif // WEBSRVR_H

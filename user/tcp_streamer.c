#include "mem.h"
#include "tcp_streamer.h"

ADD_ITEM(tcp_streamer);
DELETE_ITEM(tcp_streamer);

bool is_sending = false;

tcp_streamer *find_item(tcp_streamer *current,struct  espconn *pCon)
{
    for(;current && current->pCon!=pCon; current=current->next);
    return current;
}

void sendStringCreateStreamer(tcp_streamer **current, struct espconn* conn, const strBuf *buffer)
{
    tcp_streamer* s = add_tcp_streamer_item(current);

    s->pCon=conn;

    sendString(s,buffer);
}

void sendStringCreateStreamerNoCopy(tcp_streamer **current, struct espconn* conn, const strBuf *buffer)
{
    tcp_streamer* s = add_tcp_streamer_item(current);

    s->pCon=conn;

    sendStringNoCopy(s,buffer);
}


void sendString(tcp_streamer* s,const strBuf *buffer)
{
    if(is_sending)
    {
        s->mode=SendString;
        copy(buffer,&s->string);
    }
    else
    {
        s->mode=KillMeNoDisconnect;
        is_sending=true;
        espconn_sent(s->pCon,buffer->begin,buffer->len);
        log_free(buffer->begin);
    }
}


void sendStringNoCopy(tcp_streamer *s, const strBuf *buffer)
{
    if(is_sending)
    {
        s->mode=SendString;
        s->string=*buffer;
    }
    else
    {
        s->mode=KillMeNoDisconnect;
        is_sending=true;
        espconn_sent(s->pCon,buffer->begin,buffer->len);
        log_free(buffer->begin);
    }
}

void sendFileNoCopy(tcp_streamer *s, strBuf *buffer, uint32_t pos, uint32_t tail)
{
    if(is_sending)
    {
        s->mode=SendFile;

        s->string=*buffer;

        s->pos=pos;
        s->tail=tail;
    }
    else
    {
        s->mode=File;

        s->pos=pos;
        s->tail=tail;

        is_sending=true;

        espconn_sent(s->pCon,buffer->begin,buffer->len);

        log_free(buffer->begin);
    }
}

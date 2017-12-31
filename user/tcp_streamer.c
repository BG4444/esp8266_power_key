#include "mem.h"
#include "tcp_streamer.h"

bool is_sending = false;

tcp_streamer *add_item(tcp_streamer **current)
{
    while(*current)
    {
        current=& (*current)->next;
    }

    *current=(tcp_streamer*)os_malloc(sizeof(tcp_streamer));

    (*current)->next=0;

    return *current;
}

void delete_item(tcp_streamer **current, tcp_streamer *item)
{
    while(*current!=item)
    {
        current=& (*current)->next;
    }

    *current=(*current)->next;

    os_free(item);
}

tcp_streamer *find_item(tcp_streamer *current,struct  espconn *pCon)
{
    while( current && current->pCon!=pCon)
    {
        current=current->next;
    }
    return current;
}


void sendStringCreateStreamer(tcp_streamer **current, struct espconn* conn,char *buffer)
{
    tcp_streamer* s = add_item(current);

    s->pCon=conn;

    sendString(s,buffer);
}

void sendString(tcp_streamer* s,char *buffer)
{

    const size_t len=strlen(buffer);

    if(is_sending)
    {
        s->mode=SendString;
        s->stringBuf=os_malloc(len);
        memcpy(s->stringBuf,buffer, len);
        s->len=len;
    }
    else
    {
        s->mode=KillMe;
        is_sending=true;
        espconn_sent(s->pCon,buffer,strlen(buffer));
    }
}

void sendFile(tcp_streamer* s,char *buffer, uint32_t pos, uint32_t tail)
{
    const size_t len=strlen(buffer);

    if(is_sending)
    {
        s->mode=SendFile;

        s->stringBuf=os_malloc(len);
        memcpy(s->stringBuf,buffer, len);
        s->len=len;

        s->pos=pos;
        s->tail=tail;
    }
    else
    {
        s->mode=File;

        s->pos=pos;
        s->tail=tail;

        is_sending=true;

        espconn_sent(s->pCon,buffer,strlen(buffer));
    }
}

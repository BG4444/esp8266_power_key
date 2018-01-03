#ifndef TCP_STREAMER_H
#define TCP_STREAMER_H
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "strbuf.h"

extern bool is_sending;

typedef enum StreamMode_
{
    File,
    LongPoll,
    KillMe,
    SendString,
    SendFile
} StreamMode;

struct tcp_streamer_
{
    struct tcp_streamer_* next;

    struct espconn *pCon;

    StreamMode mode;
    ///////////////////
    strBuf string;
    ///////////////////
    uint32_t timer;
    ///////////////////
    uint32 pos;
    uint32 tail;
};

typedef struct tcp_streamer_ tcp_streamer;


tcp_streamer* add_item(tcp_streamer** current);

void delete_item(tcp_streamer** current,tcp_streamer* item);

tcp_streamer* find_item(tcp_streamer* current, struct espconn *pCon);

void sendStringCreateStreamer(tcp_streamer **current, struct espconn *conn, const strBuf *buffer);
void sendStringCreateStreamerNoCopy(tcp_streamer **current, struct espconn *conn, const strBuf *buffer);

void sendString(tcp_streamer* stream, const strBuf *buffer);
void sendStringNoCopy(tcp_streamer* stream, const strBuf *buffer);

void sendFile(tcp_streamer* s, strBuf *buffer, uint32_t pos, uint32_t tail);

#endif // TCP_STREAMER_H

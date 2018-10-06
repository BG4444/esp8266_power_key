#ifndef LOGGER_H
#define LOGGER_H

#include "linked_list.h"
#include "strbuf.h"
#include "mem.h"

typedef struct log_entry_
{
    struct log_entry_* next;

    strBuf message;
    size_t timestamp;

} log_entry;

dADD_ITEM(log_entry);
dDELETE_ITEM(log_entry);

extern log_entry* log_entries;
extern size_t nTicks;

void add_message(const strBuf* mess, const size_t time);
size_t getCurrentLength();

void registerMemoryGet(size_t size);

void add_log_buffer(const char* buf);

void* log_malloc(const size_t size);

void log_free(void* entry);


#define LOG_CLIENT_E(message,pespconn,err)  {\
                                            char buf[512];\
                                            os_sprintf(buf,message ": %u.%u.%u.%u:%u %u\n",\
                                                            pespconn->proto.tcp->remote_ip[0],\
                                                            pespconn->proto.tcp->remote_ip[1],\
                                                            pespconn->proto.tcp->remote_ip[2],\
                                                            pespconn->proto.tcp->remote_ip[3],\
                                                            pespconn->proto.tcp->remote_port,\
                                                            err\
                                                      );\
                                          add_log_buffer(buf);\
                                      }

#define LOG_CLIENT(message,pespconn)  LOG_CLIENT_E(message, pespconn, pespconn)

#endif // LOGGER_H

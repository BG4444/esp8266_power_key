#ifndef STRBUF_H
#define STRBUF_H
#include "ets_sys.h"

typedef struct _strBuf
{
    char* begin;
    unsigned short int len;
} strBuf;

bool compare(const strBuf* a, const strBuf* b);

unsigned short int find(const strBuf* url, unsigned short int pos, const char chr);

bool split(const strBuf* url, strBuf* res, const unsigned int count, const char chr);

uint32 minimum(uint32 a, uint32 b);

#endif // STRBUF_H

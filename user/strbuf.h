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

void  append(const size_t count,strBuf *ret,...);

void copy(const strBuf* from, strBuf* to);

void mid(const strBuf* from,strBuf* to, size_t begin,const size_t len);

size_t bufToInt(const strBuf* in);

void intToBuf(size_t in, strBuf* out);

size_t substr(const strBuf *url, const strBuf *sample);

#define MAKE_STR_BUF(name, content)  static const strBuf const name={content,sizeof(content)-1};

#endif // STRBUF_H

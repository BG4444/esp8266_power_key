#include "strbuf.h"
#include "ets_sys.h"
#include <string.h>
#include <mem.h>

bool split(const strBuf *url, strBuf *res, const unsigned int count, const char chr)
{
    unsigned short int pos=0;

    unsigned short int i=0;

    while(pos <= url->len && i < count)
    {
        const unsigned short tail=find(url, pos, chr);

        const strBuf temp={url->begin+pos, tail-pos};

        res[i++]=temp;
        pos=tail+1;
    }
    return pos > url->len;
}

unsigned short find(const strBuf *url, unsigned short pos, const char chr)
{
    for(; pos < url->len; ++pos)
    {
        if(url->begin[pos]==chr)
        {
            break;
        }
    }
    return pos;
}

size_t substr(const strBuf *url, const strBuf *sample)
{
    for(size_t i=0; url->len>=i+sample->len; ++i)
    {
        if(strncmp(&url->begin[i],sample->begin,sample->len)==0)
        {
            return i;
        }
    }
    return (size_t)-1;
}

bool compare(const strBuf *a, const strBuf *b)
{
    if(a->len!=b->len)
    {
        return false;
    }
    return strncmp(a->begin,b->begin,a->len)==0;
}

uint32 minimum(uint32 a, uint32 b)
{
    return (a>b) ? b : a;
}

void append(const strBuf *a, const strBuf *b, strBuf *ret)
{
    ret->len=a->len+b->len;
    ret->begin=os_malloc(ret->len);
    memcpy(ret->begin,a->begin,a->len);
    memcpy(ret->begin+a->len,b->begin,b->len);
}

void copy(const strBuf *from, strBuf *to)
{
    to->len=from->len;
    to->begin=os_malloc(to->len);
    memcpy(to->begin,from->begin,to->len);
}

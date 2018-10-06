#include "strbuf.h"
#include "ets_sys.h"
#include <string.h>
#include <mem.h>
#include <stdarg.h>

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
void copy(const strBuf *from, strBuf *to)
{
    to->len=from->len;
    to->begin=os_malloc(to->len);
    memcpy(to->begin,from->begin,to->len);
}

void mid(const strBuf *from, strBuf *to, size_t begin, const size_t len)
{
    to->begin=from->begin+begin;
    to->len=len;
}

size_t bufToInt(const strBuf *in)
{
    size_t ret=0;
    size_t mult=1;
    for(size_t pos=in->len; pos--  ; )
    {
        ret+= (in->begin[pos]-'0') * mult;
        mult=mult<<3;
    }
    return ret;
}

void intToBuf(size_t in, strBuf *out)
{
    size_t cp_in= in ? in : 1;
    for(out->len = 0; cp_in; ++out->len,cp_in/=10 );
    out->begin=log_malloc(out->len);
    for(size_t i=out->len;i--; in/=10)
    {
        out->begin[i] = (in % 10) + '0';
    }
}

void append(const size_t count, strBuf *ret, ...)
{
    ret->len=0;

    va_list list1;
    va_list list2;
    va_start(list1,ret);
    va_copy(list2,list1);

    for(size_t i=0; i<count; ++i)
    {
        strBuf* t=va_arg(list1, strBuf*);
        ret->len+=t->len;
    }

    va_end(list1);



    ret->begin=(char*)os_malloc(ret->len);


    char* pos=ret->begin;

    for(size_t i=0; i< count;++i)
    {
        strBuf* t=va_arg(list2, strBuf*);
        memcpy(pos,t->begin,t->len);
        pos+=t->len;
    }

    va_end(list2);
}

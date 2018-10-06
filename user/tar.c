#include "tar.h"
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "gpio.h"
#include "user_config.h"
#include "net_config.h"



void vaddr_flash_read512(uint32 base, uint32 *buf, uint32 size)
{
    if (base < 0x2e000)
    {
        spi_flash_read(base + 0x10000, buf, size);
    }
    else
    {
        spi_flash_read(base - 0x2e000 + 0x63000, buf, size);
    }
}

void paramToStrBuf(char* param,const size_t param_len,strBuf* out)
{
    const char* end=memccpy(out->begin,param,' ',param_len);
    out->len = end ? end-out->begin - 1 : param_len;
}

bool find_file_in_tar(const strBuf*name, uint32 *base, uint32 *size, strBuf* mtime)
{
    *base = 0;

    char buf[512];

    while(true)
    {
        vaddr_flash_read512(*base,(uint32*)buf, 512);

        tar_header* tar=(tar_header*)buf;

        bool b=false;

        for(size_t i=0; i < 512; ++i)
        {
            b = b || (buf[i]);
        }

        if(!b)
        {
            return false;
        }        

        strBuf szBuf;
        strBuf chars[sizeof(tar->size)];
        szBuf.begin=chars;

        paramToStrBuf(tar->size, sizeof(tar->size),&szBuf);

        *size=bufToInt(&szBuf);

        if(strncmp(tar->name,name->begin,minimum(
                                                    sizeof(tar->name),
                                                    name->len
                                                )
                  )==0
          )
        {
            (*base)+=512;           
            paramToStrBuf(tar->mtime, sizeof(tar->mtime),mtime);
            return true;
        }

        const uint32 addition = ((*size) % 512) ? 1 : 0;
        const uint32 fullBlockCount = (*size) / 512;

        (*base)+= 512 * ( 1 + fullBlockCount + addition );
    }
}

void send_item(tcp_streamer *s)
{
    const uint32 size=minimum(512, s->tail - s->pos);

    uint32* buf=(uint32*)log_malloc(size);

    vaddr_flash_read512(s->pos, buf, size );
    espconn_sent(s->pCon, (char*)buf,size);

    s->pos+=size;
    if(s->pos==s->tail)
    {
        s->mode=KillMeNoDisconnect;
    }

    log_free(buf);
}

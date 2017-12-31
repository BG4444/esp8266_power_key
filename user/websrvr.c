#include "websrvr.h"

#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "gpio.h"
#include "tcp_streamer.h"
#include "tar.h"

tcp_streamer* streams = 0;

static char reply404[]="HTTP/1.1 404 Not Found\r\n\r\n";
static char reply200[]="HTTP/1.1 200 OK\r\n\r\n";

void answer500(struct espconn *conn)
{
    static char reply[]="HTTP/1.1 500 Internal Server Error\r\n\r\nInternal Server Error\r\n";
    sendStringCreateStreamer(&streams,conn,reply);
}

void answer404(struct espconn *conn)
{   
    sendStringCreateStreamer(&streams,conn,reply404);
}

void answer200(struct espconn *conn)
{    
    sendStringCreateStreamer(&streams,conn,reply200);
}

bool unpackParams(const strBuf *params, Param *ret, const unsigned short count)
{
    unsigned short int i=0;
    for(;i < count; ++i)
    {
        strBuf nameValue[2];
        if(!split(&params[i], nameValue, 2, '='))
        {
            return false;
        }

        ret[i].name=nameValue[0];

        if(nameValue[1].len!=1)
        {
            return false;
        }

        const char tValue=nameValue[1].begin[0];

        if(tValue <'0' || tValue > '1')
        {
            return false;
        }
        ret[i].value=tValue-'0';
    }
    return true;
}

unsigned short findParam(const strBuf *name, const Param *params, const unsigned short count)
{
    unsigned short int i=0;
    for(; i< count; ++i)
    {
        if(compare(name,&params[i].name))
        {
            return i;
        }
    }
    return m1;
}

unsigned int getPinStatus()
{
    return ( (gpio_input_get() & BIT1) >> 1 ) | ( (gpio_input_get() & BIT3) >> 2);
}

void setPinStatus(size_t a)
{

    const size_t mask = ( (a & BIT0) << 1 ) | ( (a & BIT1) << 2);
    gpio_output_set(mask,~mask, BIT1 | BIT3 ,0);
}

void doReply(const strBuf *inputMessageAll, struct espconn* conn)
{
    const strBuf httpStr=   {"HTTP/1.1\r\n",10};
    const strBuf indexStr=    {"index.html",10};
    const strBuf statusStr=   {"status.cgi",10};
    const strBuf _404Str=     {"404.html",8};
    const strBuf switchStr=   {"/switch.cgi",11};
    const strBuf pollStr=     {"poll.cgi",8};
    const strBuf strMode =    {"mo",2};
    const strBuf strChannel = {"ch",2};
    const strBuf getStr=      {"GET",3};
    const strBuf putStr=      {"PUT",3};
    const strBuf strrnrn ={"\r\n\r\n",4};

    //находим конец заголовка запроса
    const unsigned short int  firstEOL=find(inputMessageAll,0,'\n');



    if(firstEOL==inputMessageAll->len)
    {
        answer500(conn);
        return;
    }

    const strBuf inputMessage={inputMessageAll->begin, 1+firstEOL};

    strBuf requestString[3];

    //бьем заголовок по пробелам на части 0 - команда, 1 - путь, 2 - протокол
    if(!split(&inputMessage, requestString, 3, ' '))
    {
        answer500(conn);
        return;
    }

    //проверяем, что протокол - это HTTP1.1
    if(!compare(&httpStr, &requestString[2]))
    {
        answer500(conn);
        return;
    }

    strBuf path[2];
    if(!split(&requestString[1],path,2,'/'))
    {
        answer500(conn);
        return;
    }

    if(path[0].len)
    {
        answer500(conn);
        return;
    }

    uint32 tsize;
    uint32 spos;
    bool status;

    //проверяем, что команда - это GET
    if(compare(&getStr, &requestString[0]))
    {
        if(path[1].len==0)
        {
            path[1]=indexStr;
        }

        if(compare(&path[1], &statusStr))
        {
            sendStatus(conn);
            return;
        }

        if(compare(&path[1], &pollStr))
        {
            tcp_streamer* s = add_item(&streams);

            s->pCon=conn;

            s->mode=LongPoll;
            s->timer=3;
            return;
        }
        status = find_file_in_tar(&path[1],&spos,&tsize);
    }
    else if(compare(&putStr, &requestString[0]))
    {
        if(compare(&path[1], &statusStr))
        {

            const strBuf inputBody={inputMessageAll->begin+1+firstEOL, inputMessageAll->len-1-firstEOL};
            const size_t dataStart=substr(&inputBody,&strrnrn);
            if(dataStart==(size_t)-1)
            {
                answer500(conn);
                return;
            }

            const strBuf data={inputBody.begin+dataStart+strrnrn.len,inputBody.len-dataStart-strrnrn.len};

            if(!data.len)
            {
                answer500(conn);
                return;
            }

            if(data.begin[0]<'0' || data.begin[0]>'3')
            {
                answer500(conn);
                return;
            }

            setPinStatus(data.begin[0]-'0');

            answer200(conn);

            char buffer[512];

            os_sprintf(buffer, "HTTP/1.1 200 OK\r\n\r\n"
                               "%d",
                       getPinStatus()
                       );

            for(tcp_streamer* cur=streams; cur; cur=cur->next)
            {
                if(cur->mode==LongPoll)
                {
                    sendString(cur,buffer);
                }
            }


            return;
        }
        status = false;
    }

    if(!status)
    {
        if(!find_file_in_tar(&_404Str,&spos,&tsize))
        {
            answer500(conn);
            return;
        }
    }

    tcp_streamer* s = add_item(&streams);

    s->pCon=conn;

    if(status)
    {        
        sendFile(s, reply200, spos, spos+tsize);
    }
    else
    {
        sendFile(s, reply404, spos, spos+tsize);
    }

}

void sendStatus(struct espconn *conn)
{
    char buffer[512];

    os_sprintf(buffer, "HTTP/1.1 200 OK\r\n\r\n"
                       "%d",
               getPinStatus()
               );
    sendStringCreateStreamer(&streams,conn,buffer);
}

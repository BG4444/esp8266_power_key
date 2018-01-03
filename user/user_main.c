#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "gpio.h"
#include "user_config.h"
#include "net_config.h"
#include "tcp_streamer.h"
#include "strbuf.h"
#include "websrvr.h"

static void at_tcpclient_connect_cb(void *arg);

static volatile os_timer_t WiFiLinker;

static volatile unsigned int nTicks=0;
static volatile unsigned int nClients=0;
bool connected=false;

static void ICACHE_FLASH_ATTR senddata()
{
        char info[150];
        char tcpserverip[15];
        struct espconn *pCon = (struct espconn *)os_zalloc(sizeof(struct espconn));
        if (pCon == NULL)
        {
                #ifdef PLATFORM_DEBUG
                uart0_sendStr("TCP connect failed\r\n");
                #endif
                return;
        }
        pCon->type = ESPCONN_TCP;
        pCon->state = ESPCONN_NONE;
        pCon->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
        pCon->proto.tcp->local_port = 80;
        espconn_regist_connectcb(pCon, at_tcpclient_connect_cb);
        // Можно зарегистрировать callback функцию, вызываемую при реконекте, но нам этого пока не нужно
        //espconn_regist_reconcb(pCon, at_tcpclient_recon_cb);
        // Вывод отладочной информации
        // Установить соединение с TCP-сервером
        espconn_accept(pCon);
        espconn_regist_time(pCon, 60*60, 0);
}

static void ICACHE_FLASH_ATTR wifi_check_ip(void *arg)
{        
        for(tcp_streamer* current=streams;current;current=current->next)
        {
            if(current->mode==LongPoll && --current->timer == 0)
            {
                const char status ='0'+getPinStatus();
                strBuf send;

                sendStatus(status,&send);
                sendStringNoCopy(current,&send);
            }
        }


        ++nTicks;
        // Структура с информацией о полученном, ip адресе клиента STA, маске подсети, шлюзе.
        struct ip_info ipConfig;
        // Отключаем таймер проверки wi-fi
        os_timer_disarm(&WiFiLinker);
        // Получаем данные о сетевых настройках
        wifi_get_ip_info(STATION_IF, &ipConfig);
        // Проверяем статус wi-fi соединения и факт получения ip адреса
        if (wifi_station_get_connect_status() == STATION_GOT_IP && ipConfig.ip.addr != 0)
        {
                // Соединения по wi-fi установлено
//                connState = WIFI_CONNECTED;
                #ifdef PLATFORM_DEBUG
                uart0_sendStr("WiFi connected\r\n");
        #endif
                #ifdef PLATFORM_DEBUG
                uart0_sendStr("Start TCP connecting...\r\n");
        #endif

                if(!connected)
                {
                    senddata();
                }
                connected=true;
                // Запускаем таймер проверки соединения и отправки данных уже раз в 5 сек, см. тех.задание
                os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
                os_timer_arm(&WiFiLinker, 5000, 0);
        }
        else
        {
                connected=false;
                // Неправильный пароль
                if(wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
                {
//                        connState = WIFI_CONNECTING_ERROR;
                        #ifdef PLATFORM_DEBUG
                        uart0_sendStr("WiFi connecting error, wrong password\r\n");
                        #endif
                        os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
                        os_timer_arm(&WiFiLinker, 1000, 0);
                }
                // AP не найдены
                else if(wifi_station_get_connect_status() == STATION_NO_AP_FOUND)
                {
//                        connState = WIFI_CONNECTING_ERROR;
                        #ifdef PLATFORM_DEBUG
                        uart0_sendStr("WiFi connecting error, ap not found\r\n");
                        #endif
                        os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
                        os_timer_arm(&WiFiLinker, 1000, 0);
                }
                // Ошибка подключения к AP
                else if(wifi_station_get_connect_status() == STATION_CONNECT_FAIL)
                {
//                        connState = WIFI_CONNECTING_ERROR;
                        #ifdef PLATFORM_DEBUG
                        uart0_sendStr("WiFi connecting fail\r\n");
                        #endif
                        os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
                        os_timer_arm(&WiFiLinker, 1000, 0);
                }
                // Другая ошибка
                else
                {
                        os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
                        os_timer_arm(&WiFiLinker, 1000, 0);
//                        connState = WIFI_CONNECTING;
                        #ifdef PLATFORM_DEBUG
                        uart0_sendStr("WiFi connecting...\r\n");
                        #endif
                }
        }
}

static void ICACHE_FLASH_ATTR at_tcpclient_sent_cb(void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;

    tcp_streamer* cur=find_item(streams,pespconn);

    if(cur)
    {
        switch(cur->mode)
        {
            case File:
                send_item(cur);
            break;
            case KillMe:
                delete_item(&streams, cur);
                espconn_disconnect(pespconn);

                tcp_streamer* current=streams;
                for(; current; current=current->next)
                {
                    if(current->mode == SendString || current->mode == SendFile)
                    {
                        if(current->mode==SendString)
                        {
                            current->mode=KillMe;
                        }
                        else
                        {
                            current->mode=File;
                        }
                        espconn_sent(current->pCon, current->string.begin, current->string.len);
                        os_free(current->string.begin);
                        break;
                    }
                }
                if(!current)
                {
                    is_sending=false;
                }
            break;
        }
    }
}


static void ICACHE_FLASH_ATTR at_tcpclient_recv_cb(void *arg,char *pdata, unsigned short len)
{
    struct espconn *pespconn = (struct espconn *)arg;

    const strBuf inputMessage={pdata,len};

    bool changeMode;
    unsigned char mode;
    unsigned char channel;

    doReply(&inputMessage,pespconn);

//    switch(doReply(&inputMessage,&changeMode,&channel,&mode))
//    {
//        case 0:
//            answer500(pespconn);
//            espconn_disconnect(pespconn);
//        break;
//        case 3:
//        case 1:
//            answer200(pespconn);
//            espconn_disconnect(pespconn);
//        break;
//        case 2:
//            answerj0(pespconn);
//        break;
//    }


//    if(changeMode)
//    {
//        const unsigned char bitno = channel ? BIT3 : BIT1;
//        if(mode)
//        {
//            gpio_output_set(bitno, 0, bitno, 0);
//        }
//        else
//        {
//            gpio_output_set(0, bitno, bitno, 0);
//        }
//    }
}


static void ICACHE_FLASH_ATTR at_tcpclient_discon_cb(void *arg)
{
        struct espconn *pespconn = (struct espconn *)arg;
        // Отключились, освобождаем память
       // os_free(pespconn->proto.tcp);
       // os_free(pespconn);
        #ifdef PLATFORM_DEBUG
        uart0_sendStr("Disconnect callback\r\n");
        #endif

        --nClients;
}



static void ICACHE_FLASH_ATTR at_tcpclient_connect_cb(void *arg)
{
        struct espconn *pespconn = (struct espconn *)arg;
        #ifdef PLATFORM_DEBUG
        uart0_sendStr("TCP client connect\r\n");
        #endif

        espconn_regist_recvcb(pespconn, at_tcpclient_recv_cb);

        // callback функция, вызываемая после отправки данных
        espconn_regist_sentcb(pespconn, at_tcpclient_sent_cb);

        // callback функция, вызываемая после отключения
        espconn_regist_disconcb(pespconn, at_tcpclient_discon_cb);
        char payload[512];
        ++nClients;
        // Подготавливаем строку данных, будем отправлять MAC адрес ESP8266 в режиме AP и добавим к нему строку ESP8266
        os_sprintf(payload, "TIME: %ld ncients: %ld LinkCnt: %ld\r\n", nTicks, nClients, pespconn->link_cnt                   );
        #ifdef PLATFORM_DEBUG
        uart0_sendStr(payload);
        #endif
        // Отправляем данные
       // espconn_sent(pespconn, payload, strlen(payload));
}


void BtnInit()
{
    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);

    //Set GPIO2 low
    gpio_output_set(0, BIT1, BIT1, 0);
    gpio_output_set(0, BIT3, BIT3, 0);

}

//Init function 
void ICACHE_FLASH_ATTR user_init()
{
            #ifdef PLATFORM_DEBUG
            // Вывод строки в uart о начале запуска, см. определение PLATFORM_DEBUG в user_config.h
            uart0_sendStr("ESP8266 platform starting...\r\n");
            #endif
            // Структура с информацией о конфигурации STA (в режиме клиента AP)
            struct station_config stationConfig;
            char info[150];
            // Проверяем если платы была не в режиме клиента AP, то переводим её в этот режим
            // В версии SDK ниже 0.9.2 после wifi_set_opmode нужно было делать system_restart
            if(wifi_get_opmode() != STATION_MODE)
            {
                    #ifdef PLATFORM_DEBUG
                    uart0_sendStr("ESP8266 not in STATION mode, restarting in STATION mode...\r\n");
                    #endif
                    wifi_set_opmode(STATION_MODE);
            }
            // Если плата в режиме STA, то устанавливаем конфигурацию, имя AP, пароль, см. user_config.h
            // Дополнительно читаем MAC адрес нашей платы для режима AP, см. wifi_get_macaddr(SOFTAP_IF, macaddr);
            // В режиме STA у платы будет другой MAC адрес, как у клиента, но мы для себя читаем адрес который у неё был если бы она выступала как точка доступа
            if(wifi_get_opmode() == STATION_MODE)
            {
                    wifi_station_get_config(&stationConfig);
                    os_memset(stationConfig.ssid, 0, sizeof(stationConfig.ssid));
                    os_memset(stationConfig.password, 0, sizeof(stationConfig.password));
                    os_sprintf(stationConfig.ssid, "%s", WIFI_CLIENTSSID);
                    os_sprintf(stationConfig.password, "%s", WIFI_CLIENTPASSWORD);
                    wifi_station_set_config(&stationConfig);
            }
            // Для отладки выводим в uart данные о настройке режима STA
            #ifdef PLATFORM_DEBUG
            if(wifi_get_opmode() == STATION_MODE)
            {
                    wifi_station_get_config(&stationConfig);
                    os_sprintf(info,"OPMODE: %u, SSID: %s, PASSWORD: %s\r\n",
                            wifi_get_opmode(),
                            stationConfig.ssid,
                            stationConfig.password);
                    uart0_sendStr(info);
            }
            #endif

            // Запускаем таймер проверки соединения по Wi-Fi, проверяем соединение раз в 1 сек., если соединение установлено, то запускаем TCP-клиент и отправляем тестовую строку.
            os_timer_disarm(&WiFiLinker);
            os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
            os_timer_arm(&WiFiLinker, 1000, 0);
            // Инициализируем GPIO,
            BtnInit();
            // Выводим сообщение о успешном запуске
            #ifdef PLATFORM_DEBUG
            uart0_sendStr("ESP8266 platform started!\r\n");
            #endif
}

/*
==================================================
                天气数据获取模块
==================================================
功能：
    通过 USART2 与 Air780E 4G 模块通信
    发送 AT 指令发起 HTTP GET 请求
    从心知天气 API 获取郑州实时天气
    解析 JSON 响应提取城市、天气、温度

依赖：
    USART2  - 与 Air780E 模块的串口通信
    delay   - 延时等待模块响应
==================================================
*/
#include "weather.h"
#include "usart.h"
#include "delay.h"
#include <string.h>
#include <stdio.h>



/*====================================================
    天气数据全局实例

    存储 API 返回的城市、天气描述和温度
    供 UI 模块读取显示
====================================================*/

WeatherData weather;



/*====================================================
    等待Air780E返回

    在指定超时时间内检查 USART2 接收缓冲区
    是否包含期望的字符串

    参数：
        str     : 期望匹配的字符串指针
        timeout : 超时时间（单位：毫秒）

    返回值：
        0 : 成功，在超时前匹配到字符串
        1 : 超时，未匹配到字符串
====================================================*/

static u8 Weather_WaitCheck(const char *str,u16 timeout)
{

    u16 cnt=0;


    /* 循环检测直到超时 */
    while(cnt<timeout)
    {

        /* 检查接收缓冲区是否包含期望字符串 */
        if(strstr(USART2_RX_BUF,str)!=NULL)
        {
            return 0;   /* 匹配成功 */
        }


        delay_ms(1);   /* 每次检测间隔 1ms */

        cnt++;

    }


    return 1;   /* 超时未匹配 */

}





/*====================================================
    JSON解析

    从 USART2 接收缓冲区中解析天气 JSON 数据

    解析字段:
        name        : 城市名称
        text        : 天气描述
        temperature : 室外温度

    返回值:
        0 : 解析成功
        1 : 未找到 JSON 起始标记
        2 : 未找到城市字段
        3 : 城市字段解析异常
        4 : 未找到天气字段
        5 : 天气字段解析异常
        6 : 未找到温度字段
        7 : 温度字段解析异常
====================================================*/

static u8 Weather_Parse(void)
{
    char *json;
    char *p;
    char *start;
    char *end;


    /*
        找JSON开始位置

        在接收缓冲区中查找 JSON 数据的起始标记 "{\"results"
        如果未找到说明响应格式错误
    */

    json = strstr(USART2_RX_BUF,"{\"results\"");


    if(json == NULL)
    {
        return 1;   /* 未找到 JSON 数据 */
    }



    /*
        解析城市

        在 JSON 中查找 "name" 字段
        提取引号内的城市名称字符串
    */

    p = strstr(json,"\"name\":\"");   /* 查找城市字段起始位置 */


    if(p == NULL)
    {
        return 2;
    }


    start = p + strlen("\"name\":\"");   /* 跳过字段名，定位到值起始 */


    end = strchr(start,'"');   /* 查找值的结束引号 */


    if(end == NULL)
    {
        return 3;
    }


    /* 清空城市缓冲区，防止旧数据残留 */
    memset(
        weather.city,
        0,
        sizeof(weather.city)
    );


    /* 复制城市名称到 weather.city */
    memcpy(
        weather.city,
        start,
        end-start
    );




    /*
        解析天气

        在 JSON 中查找 "text" 字段
        提取引号内的天气描述字符串
    */


    p = strstr(json,"\"text\":\"");   /* 查找天气字段起始位置 */


    if(p == NULL)
    {
        return 4;
    }


    start = p + strlen("\"text\":\"");   /* 跳过字段名，定位到值起始 */


    end = strchr(start,'"');   /* 查找值的结束引号 */


    if(end == NULL)
    {
        return 5;
    }


    /* 清空天气缓冲区，防止旧数据残留 */
    memset(
        weather.text,
        0,
        sizeof(weather.text)
    );


    /* 复制天气描述到 weather.text */
    memcpy(
        weather.text,
        start,
        end-start
    );





    /*
        解析温度

        在 JSON 中查找 "temperature" 字段
        提取引号内的温度字符串
    */


    p = strstr(json,"\"temperature\":\"");   /* 查找温度字段起始位置 */


    if(p == NULL)
    {
        return 6;
    }



    start = p + strlen("\"temperature\":\"");   /* 跳过字段名，定位到值起始 */



    end = strchr(start,'"');   /* 查找值的结束引号 */


    if(end == NULL)
    {
        return 7;
    }



    /* 清空温度缓冲区，防止旧数据残留 */
    memset(
        weather.temperature,
        0,
        sizeof(weather.temperature)
    );



    /* 复制温度字符串到 weather.temperature */
    memcpy(
        weather.temperature,
        start,
        end-start
    );



    return 0;   /* 解析成功 */
}





/*====================================================
    获取天气

    完整流程：
        1. 关闭旧 HTTP 会话
        2. 配置 GPRS 数据连接
        3. 打开数据连接
        4. HTTP 初始化并设置 URL
        5. 发起 HTTP GET 请求
        6. 读取响应数据
        7. 解析 JSON 提取天气信息
        8. 释放 HTTP 会话

    城市:
        Zhengzhou（郑州）

    语言:
        English

    返回值:
        0 : 成功
        1 : HTTP ACTION 请求失败
        2 : 响应中未找到 JSON 数据
        3 : JSON 解析失败
====================================================*/
u8 Weather_Get(void)
{

    u8 ret;


    memset(
        &weather,
        0,
        sizeof(weather)
    );



    /*
        关闭旧HTTP状态

        AT+HTTPTERM 关闭可能存在的旧 HTTP 会话
        无论成功失败都继续
    */

    USART2_Clear();

    USART2_SendString(
        "AT+HTTPTERM\r\n"
    );

    delay_ms(500);



    /*
        等待4G网络注册

        上电后Air780E需要时间注册网络，
        在GPRS未附着时激活bearer会失败（返回+SAPBR: 1,3）。
        循环检测AT+CGATT?直到返回+CGATT: 1
        最多等待15秒。
    */

    {
        u8 retry;

        for(retry = 0; retry < 15; retry++)
        {
            USART2_Clear();

            USART2_SendString(
                "AT+CGATT?\r\n"
            );

            delay_ms(500);

            if(strstr(USART2_RX_BUF, "+CGATT: 1") != NULL)
            {
                printf("GPRS ATTACHED\r\n");
                break;
            }

            printf("Waiting GPRS... (%d)\r\n", retry + 1);

            delay_ms(500);
        }
    }



    /*
        GPRS配置

        设置承载参数类型为 GPRS
        设置 APN 为空（使用运营商默认）
    */


    USART2_Clear();

    /* 设置连接类型为 GPRS */
    USART2_SendString(
        "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n"
    );

    Weather_WaitCheck(
        "OK",
        3000
    );




    USART2_Clear();

    /* 设置 APN 为空，使用运营商默认配置 */
    USART2_SendString(
        "AT+SAPBR=3,1,\"APN\",\"\"\r\n"
    );

    Weather_WaitCheck(
        "OK",
        3000
    );




    /*
        打开数据连接（带重试）

        AT+SAPBR=1,1 打开 GPRS 承载
        AT+SAPBR=2,1 查询承载状态

        +SAPBR: 1,1,"x.x.x.x" 表示已激活
        +SAPBR: 1,3,"0.0.0.0" 表示未激活

        如果激活失败，重试最多3次
    */

    {
        u8 br_retry;

        for(br_retry = 0; br_retry < 3; br_retry++)
        {
            USART2_Clear();

            /* 打开 GPRS 数据连接 */
            USART2_SendString(
                "AT+SAPBR=1,1\r\n"
            );

            Weather_WaitCheck(
                "OK",
                10000
            );


            /* 查询承载连接状态 */
            USART2_Clear();

            USART2_SendString(
                "AT+SAPBR=2,1\r\n"
            );

            Weather_WaitCheck(
                "OK",
                3000
            );

            /* 检查是否已激活：+SAPBR: 1,1, 表示有IP地址 */
            if(strstr(USART2_RX_BUF, "+SAPBR: 1,1,") != NULL)
            {
                printf("BEARER ACTIVATED\r\n");
                break;
            }

            /* 如果返回+SAPBR: 1,1表示已经打开，也算成功 */
            if(strstr(USART2_RX_BUF, "ERROR") != NULL && br_retry == 0)
            {
                /* 第一次报ERROR可能是bearer已打开，查询确认 */
                USART2_Clear();
                USART2_SendString("AT+SAPBR=2,1\r\n");
                Weather_WaitCheck("OK", 3000);

                if(strstr(USART2_RX_BUF, "+SAPBR: 1,1,") != NULL)
                {
                    printf("BEARER ALREADY ACTIVE\r\n");
                    break;
                }
            }

            printf("BEARER RETRY... (%d)\r\n", br_retry + 1);

            delay_ms(1000);
        }
    }





    /*
        HTTP初始化

        AT+HTTPINIT  初始化 HTTP 服务
        AT+HTTPPARA  设置 HTTP 参数（CID）
    */


    USART2_Clear();

    /* 初始化 HTTP 服务 */
    USART2_SendString(
        "AT+HTTPINIT\r\n"
    );


    Weather_WaitCheck(
        "OK",
        3000
    );




    USART2_Clear();

    /* 设置 HTTP 承载 CID 为 1 */
    USART2_SendString(
        "AT+HTTPPARA=\"CID\",1\r\n"
    );


    Weather_WaitCheck(
        "OK",
        3000
    );






    /*
        设置天气URL

        心知天气 API v3
        key      : API 密钥
        location : 城市名（zhengzhou = 郑州）
        language : en（英文，便于后续中文翻译映射）
    */


    USART2_Clear();


    USART2_SendString(
        "AT+HTTPPARA=\"URL\","
        "\"http://api.seniverse.com/v3/weather/now.json?"
        "key=SM-pVlVONy0c4s2Dp"
        "&location=zhengzhou"
        "&language=en\"\r\n"
    );


    Weather_WaitCheck(
        "OK",
        3000
    );





    /*
        HTTP GET

        AT+HTTPACTION=0 发起 GET 请求
        等待响应 "+HTTPACTION: 0,200" 表示成功
        超时 15 秒（网络请求可能较慢）
    */


    USART2_Clear();


    USART2_SendString(
        "AT+HTTPACTION=0\r\n"
    );


    /* 检查 HTTP 响应状态码是否为 200（成功） */
    if(
        Weather_WaitCheck(
            "+HTTPACTION: 0,200",
            15000
        )
    )
    {

        printf("HTTP ACTION FAIL\r\n");

        return 1;
    }





    /*
        读取JSON

        AT+HTTPREAD 读取 HTTP 响应体内容
        延时 800ms 等待数据完整接收
        通过串口打印原始响应便于调试
    */


    USART2_Clear();


    USART2_SendString(
        "AT+HTTPREAD\r\n"
    );


    delay_ms(800);



    printf("\r\n========== HTTP BUFFER ==========\r\n");

    printf("%s\r\n",USART2_RX_BUF);

    printf("=================================\r\n");




    /*
        JSON判断

        检查响应缓冲区中是否包含 JSON 数据起始标记
        如果没有 JSON 数据，说明请求失败
    */

    if(
        strstr(
            USART2_RX_BUF,
            "{\"results\""
        )
        ==NULL
    )
    {

        printf(
            "JSON NOT FOUND\r\n"
        );

        return 2;

    }





    /*
        数据解析

        调用 Weather_Parse() 解析 JSON 数据
        返回非 0 表示解析失败
    */


    ret = Weather_Parse();


    if(ret != 0)
    {

        printf(
            "Weather Parse Error:%d\r\n",
            ret
        );


        return 3;

    }





    /*
        释放HTTP

        AT+HTTPTERM 关闭 HTTP 会话
        释放 Air780E 模块的 HTTP 资源
    */

    USART2_Clear();


    USART2_SendString(
        "AT+HTTPTERM\r\n"
    );


    delay_ms(200);



    return 0;

}




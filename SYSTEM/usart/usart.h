/*
==================================================
    串口驱动头文件 (usart.h)

    功能：
    定义USART1和USART2的初始化、发送、接收相关函数和变量。
    支持printf重定向到USART1用于调试输出。

    硬件连接：
    - USART1 (PA9/PA10)  : 调试串口，115200bps
    - USART2 (PA2/PA3)   : Air780E 4G模块通信，115200bps

    USART1用途：printf调试输出、接收调试命令
    USART2用途：与Air780E通信，发送AT指令，接收MQTT/HTTP响应
==================================================
*/
#ifndef __USART_H
#define __USART_H

#include "sys.h"
#include <string.h>
#include <stdio.h>


/*
    USART1
    调试串口
    接收缓存长度定义(字节)
*/
#define USART_REC_LEN 200



/*
    USART2
    Air780E通信

    MQTT:
    OneNET上传

    HTTP:
    天气API

    接收缓存长度定义(字节)
    较大以容纳AT指令的完整响应
*/
#define USART2_REC_LEN 1024



/* USART1接收缓存，存储从调试串口接收到的数据 */
extern u8 USART_RX_BUF[USART_REC_LEN];

/*
    USART1接收状态标志
    bit15: 接收完成标志(0x8000)
    bit14: 接收到0x0D回车标志(0x4000)
    bit13~0: 接收到的数据计数
*/
extern u16 USART_RX_STA;



/* USART2接收缓存，存储从Air780E接收到的AT指令响应数据 */
extern char USART2_RX_BUF[USART2_REC_LEN];

/* USART2接收数据长度，记录当前缓存中有效数据的字节数 */
extern u16 USART2_RX_LEN;



/*
    USART1初始化

    PA9  TX (复用推挽输出)
    PA10 RX (浮空输入)
    波特率：115200，8数据位，1停止位，无校验
    参数：无
    返回值：无
*/
void usart_init(void);



/*
    USART2初始化

    PA2 TX (复用推挽输出)
    PA3 RX (浮空输入)

    Air780E
    波特率：115200，8数据位，1停止位，无校验
    参数：无
    返回值：无
*/
void usart2_init(void);



/*
    通用发送函数

    USART1/USART2均可使用
    功能：逐字节发送字符串，直到遇到'\0'结束
    参数：USARTx - 串口外设(USART1或USART2)
         DataString - 待发送的字符串指针
    返回值：无
*/
void USART_SendString(USART_TypeDef* USARTx,
                      const char *DataString);



/*
    USART2发送字符串

    Air780E AT指令

    MQTT / HTTP共用
    功能：封装USART_SendString，专门用于USART2发送AT指令
    参数：str - 待发送的字符串指针
    返回值：无
*/
void USART2_SendString(const char *str);



/*
    清空USART2接收缓存
    功能：关闭中断->清空缓存->重置长度->开启中断
    参数：无
    返回值：无

    说明：
    先关闭接收中断防止清空过程中有新数据写入，
    清空后再重新开启中断。
*/
void USART2_Clear(void);



#endif


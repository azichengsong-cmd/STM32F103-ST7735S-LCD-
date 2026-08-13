#ifndef __RS485_H
#define __RS485_H

#include "sys.h"


/*================================================
 * RS485方向控制
 *
 * 通过控制PC8引脚的电平来切换MAX3485
 * 收发芯片的发送/接收方向。
 *================================================*/

#define RS485_EN_PORT      GPIOC    /* 方向控制引脚所在的GPIO端口 */
#define RS485_EN_PIN       GPIO_Pin_8 /* 方向控制引脚号 */


/* 发送模式：PC8置高，MAX3485进入发送状态 */
#define RS485_TX_EN() \
    GPIO_SetBits( \
        RS485_EN_PORT, \
        RS485_EN_PIN \
    )


/* 接收模式：PC8置低，MAX3485进入接收状态 */
#define RS485_RX_EN() \
    GPIO_ResetBits( \
        RS485_EN_PORT, \
        RS485_EN_PIN \
    )


/*================================================
 * UART4接收缓冲区
 *
 * RS485_RX_BUF_SIZE: 接收缓冲区最大容量（字节）
 *                    超过此长度的数据将被丢弃并重新接收。
 *================================================*/

#define RS485_RX_BUF_SIZE  64


/* 接收数据缓冲区，由UART4中断自动填充 */
extern volatile u8 RS485_RX_BUF[
    RS485_RX_BUF_SIZE
];

/* 当前缓冲区中已接收的数据长度 */
extern volatile u16 RS485_RX_LEN;


/*================================================
 * RS485初始化
 *
 * 初始化UART4及GPIO，开启接收中断
 *================================================*/

void RS485_Init(void);


/*================================================
 * RS485发送
 *
 * RS485_SendByte: 发送单个字节（阻塞）
 * RS485_SendData: 发送一段数据，自动切换收发方向
 *================================================*/

void RS485_SendByte(
    u8 data
);

void RS485_SendData(
    u8 *buf,
    u16 len
);


/*================================================
 * RS485接收
 *
 * 旧测试函数保留，Modbus通信不使用这些阻塞函数
 *
 * RS485_ReceiveByte: 阻塞接收单个字节
 * RS485_ReceiveData: 轮询接收一段数据
 *================================================*/

u8 RS485_ReceiveByte(void);

u16 RS485_ReceiveData(
    u8 *buf,
    u16 max_len
);


#endif


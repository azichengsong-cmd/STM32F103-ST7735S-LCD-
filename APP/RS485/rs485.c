#include "rs485.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"


/*================================================
 * RS485接收缓冲区
 *
 * RS485_RX_BUF: 接收数据缓冲区，由UART4中断
 *               服务函数自动填充。
 *
 * RS485_RX_LEN: 当前缓冲区中已接收的数据长度，
 *               每收到一个字节自动递增。
 *================================================*/

volatile u8 RS485_RX_BUF[RS485_RX_BUF_SIZE];

volatile u16 RS485_RX_LEN = 0;


/**
 * @brief  RS485初始化
 *
 * 功能说明：
 *     初始化UART4及其对应GPIO，配置波特率9600，
 *     开启接收中断，默认进入接收模式。
 *
 * UART4:
 * PC10 -> UART4_TX
 * PC11 -> UART4_RX
 *
 * PC8  -> MAX3485 DE/RE#
 *
 * PC8 = 1：发送
 * PC8 = 0：接收
 *
 * @param  无
 * @return 无
 */
void RS485_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;


    /*================================================
     * 1. 开启时钟
     *================================================*/

    /* GPIOC时钟 */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOC,
        ENABLE
    );

    /* UART4时钟 */
    RCC_APB1PeriphClockCmd(
        RCC_APB1Periph_UART4,
        ENABLE
    );


    /*================================================
     * 2. 配置PC8
     *
     * PC8 -> MAX3485 DE/RE#
     *================================================*/

    GPIO_InitStructure.GPIO_Pin =
        GPIO_Pin_8;

    GPIO_InitStructure.GPIO_Mode =
        GPIO_Mode_Out_PP;

    GPIO_InitStructure.GPIO_Speed =
        GPIO_Speed_50MHz;

    GPIO_Init(
        GPIOC,
        &GPIO_InitStructure
    );


    /*================================================
     * 3. 配置PC10
     *
     * PC10 -> UART4_TX
     *================================================*/

    GPIO_InitStructure.GPIO_Pin =
        GPIO_Pin_10;

    GPIO_InitStructure.GPIO_Mode =
        GPIO_Mode_AF_PP;

    GPIO_InitStructure.GPIO_Speed =
        GPIO_Speed_50MHz;

    GPIO_Init(
        GPIOC,
        &GPIO_InitStructure
    );


    /*================================================
     * 4. 配置PC11
     *
     * PC11 -> UART4_RX
     *================================================*/

    GPIO_InitStructure.GPIO_Pin =
        GPIO_Pin_11;

    GPIO_InitStructure.GPIO_Mode =
        GPIO_Mode_IN_FLOATING;

    GPIO_Init(
        GPIOC,
        &GPIO_InitStructure
    );


    /*================================================
     * 5. 配置UART4
     *================================================*/

    USART_InitStructure.USART_BaudRate =
        9600;                           /* 波特率9600，与上位机保持一致 */

    USART_InitStructure.USART_WordLength =
        USART_WordLength_8b;            /* 8位数据位 */

    USART_InitStructure.USART_StopBits =
        USART_StopBits_1;               /* 1位停止位 */

    USART_InitStructure.USART_Parity =
        USART_Parity_No;                /* 无校验位 */

    USART_InitStructure.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None; /* 无硬件流控 */

    USART_InitStructure.USART_Mode =
        USART_Mode_Rx |                 /* 接收模式 */
        USART_Mode_Tx;                  /* 发送模式 */

    USART_Init(
        UART4,
        &USART_InitStructure
    );


    /*================================================
     * 6. 清空接收缓冲区
     *================================================*/

    RS485_RX_LEN = 0;


    /*================================================
     * 7. 默认进入接收模式
     *================================================*/

    RS485_RX_EN();


    /*================================================
     * 8. 配置UART4中断优先级
     *================================================*/

    NVIC_InitStructure.NVIC_IRQChannel =
        UART4_IRQn;

    /*
     * 这里优先级给1
     *
     * 你的USART2也有中断，
     * 后续如果需要可以再统一调整优先级。
     */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
        1;

    NVIC_InitStructure.NVIC_IRQChannelSubPriority =
        1;

    NVIC_InitStructure.NVIC_IRQChannelCmd =
        ENABLE;

    NVIC_Init(
        &NVIC_InitStructure
    );


    /*================================================
     * 9. 开启UART4接收中断
     *
     * UART4每收到一个字节：
     *
     * RXNE = 1
     *
     * 自动进入：
     *
     * UART4_IRQHandler()
     *================================================*/

    USART_ITConfig(
        UART4,
        USART_IT_RXNE,
        ENABLE
    );


    /*================================================
     * 10. 使能UART4
     *================================================*/

    USART_Cmd(
        UART4,
        ENABLE
    );
}


/**
 * @brief  RS485发送一个字节
 *
 * 功能说明：
 *     阻塞等待发送数据寄存器为空后，写入一个字节到UART4。
 *
 * @param  data 要发送的数据
 * @return 无
 */
void RS485_SendByte(u8 data)
{
    /* 等待发送数据寄存器为空 */
    while(
        USART_GetFlagStatus(
            UART4,
            USART_FLAG_TXE
        ) == RESET
    );

    /* 发送数据 */
    USART_SendData(
        UART4,
        data
    );
}


/**
 * @brief  RS485发送一段数据
 *
 * 功能说明：
 *     切换MAX3485为发送模式，逐字节发送数据，
 *     等待最后一个字节发送完成后切换回接收模式。
 *
 * @param  buf 数据缓冲区
 * @param  len 数据长度
 * @return 无
 */
void RS485_SendData(u8 *buf, u16 len)
{
    u16 i;


    /*================================================
     * 1. 切换到发送模式
     *================================================*/

    RS485_TX_EN();


    /*================================================
     * 2. 逐字节发送
     *================================================*/

    for(i = 0; i < len; i++)
    {
        RS485_SendByte(
            buf[i]
        );
    }


    /*================================================
     * 3. 等待最后一个字节真正发送完成
     *
     * TC：
     * 整个数据已经发送完成
     *================================================*/

    while(
        USART_GetFlagStatus(
            UART4,
            USART_FLAG_TC
        ) == RESET
    );


    /*================================================
     * 4. 发送完成
     *
     * 切换回接收模式
     *================================================*/

    RS485_RX_EN();
}


/**
 * @brief  RS485接收一个字节
 *
 * 功能说明：
 *     阻塞等待UART4接收数据寄存器非空后读取一个字节。
 *
 * @return 接收到的数据
 *
 * 注意：
 * 这个函数仍然保留给其他测试代码使用。
 *
 * Modbus正式通信不再使用这个阻塞函数。
 */
u8 RS485_ReceiveByte(void)
{
    while(
        USART_GetFlagStatus(
            UART4,
            USART_FLAG_RXNE
        ) == RESET
    );

    return (
        u8
    )USART_ReceiveData(
        UART4
    );
}


/**
 * @brief  RS485接收一段数据
 *
 * 功能说明：
 *     轮询读取UART4接收到的数据，直到缓冲区填满或无数据可读。
 *
 * @param  buf     接收缓冲区
 * @param  max_len 最大接收长度
 *
 * @return 实际接收到的数据长度
 *
 * 注意：
 * Modbus正式通信不再使用这个函数。
 */
u16 RS485_ReceiveData(
    u8 *buf,
    u16 max_len
)
{
    u16 len = 0;


    while(len < max_len)
    {
        if(
            USART_GetFlagStatus(
                UART4,
                USART_FLAG_RXNE
            ) == RESET
        )
        {
            break;
        }


        buf[len] =
            (u8)USART_ReceiveData(
                UART4
            );

        len++;
    }


    return len;
}


/**
 * @brief  UART4中断服务函数
 *
 * 功能说明：
 *     UART4接收到一个字节后进入这里。
 *     仅负责将数据保存到接收缓冲区，不做协议解析。
 *
 * 这里只负责：
 *
 *     读取UART4数据
 *     保存到RS485_RX_BUF
 *
 * 不在中断里面做：
 *
 *     CRC
 *     Modbus解析
 *     printf
 *     RS485发送
 *
 * 这些工作交给主循环中的Modbus_Process()。
 *
 * @param  无
 * @return 无
 */
void UART4_IRQHandler(void)
{
    u8 data;


    /*================================================
     * 判断是否为接收中断
     *================================================*/

    if(
        USART_GetITStatus(
            UART4,
            USART_IT_RXNE
        ) != RESET
    )
    {
        /*
         * 读取UART4数据寄存器
         *
         * 读取后RXNE会被清除
         */
        data =
            (u8)USART_ReceiveData(
                UART4
            );


        /*
         * 保存到接收缓冲区
         *
         * 检查缓冲区是否已满，未满则存入数据并递增长度
         */
        if(
            RS485_RX_LEN
            < RS485_RX_BUF_SIZE
        )
        {
            RS485_RX_BUF[
                RS485_RX_LEN
            ] = data;               /* 将接收到的字节存入缓冲区 */

            RS485_RX_LEN++;         /* 接收长度加1 */
        }
        else
        {
            /*
             * 缓冲区满
             *
             * 重新开始接收
             *
             * 注意：
             * 这里不printf，
             * 不做复杂处理。
             */
            RS485_RX_LEN = 0;       /* 缓冲区满，重置长度从头开始接收 */
        }
    }
}


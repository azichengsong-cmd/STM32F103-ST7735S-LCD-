/*
==================================================
    串口驱动源文件 (usart.c)

    功能：
    实现USART1(调试串口)和USART2(Air780E通信)的初始化、
    数据发送、中断接收功能。
    包含printf重定向支持，可将printf输出重定向到USART1。

    硬件连接：
    - USART1 : PA9(TX) / PA10(RX) - 调试串口
    - USART2 : PA2(TX) / PA3(RX)  - Air780E 4G模块

    接收机制：
    - USART1：中断接收，以回车换行(0x0D 0x0A)为结束标志
    - USART2：中断接收，存入环形缓存，同时转发到USART1用于调试
==================================================
*/
#include "usart.h"



/*
    printf重定向到USART1

    PC调试使用
    说明：
    通过重写fputc函数，将printf的输出重定向到USART1。
    需要关闭半主机模式(semihosting)，
    使用#pragma import(__use_no_semihosting)实现。
*/

#if 1

#pragma import(__use_no_semihosting)


/* 标准库需要的FILE结构体，用于支持printf */
struct __FILE
{
    int handle;
};


/* 标准库需要的标准输出文件句柄 */
FILE __stdout;



/* 系统退出函数，半主机模式关闭后需要提供的桩函数 */
void _sys_exit(int x)
{
    x=x;
}



/*
    函数名：fputc
    功能：重定向fputc到USART1，使printf输出到调试串口
    参数：ch - 待输出的字符
         f - 文件指针(未使用)
    返回值：输出的字符

    说明：
    等待USART1发送数据寄存器为空(TC标志)，
    然后将字符写入数据寄存器(DR)发送出去。
*/
int fputc(int ch, FILE *f)
{

    while((USART1->SR&0X40)==0);  /* 等待发送完成标志(TC)置位 */

    USART1->DR=(u8)ch;  /* 将字符写入数据寄存器，触发发送 */


    return ch;

}

#endif




/*
    USART1初始化

    调试串口
    功能：初始化USART1用于调试输出和命令接收
    参数：无
    返回值：无

    配置：
    - 引脚：PA9(TX复用推挽), PA10(RX浮空输入)
    - 波特率：115200
    - 数据位：8位
    - 停止位：1位
    - 校验：无
    - 中断：接收中断(RXNE)，抢占优先级3，子优先级3
*/

void usart_init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;



    /* 使能GPIOA和USART1时钟(USART1挂载在APB2总线上) */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA|
        RCC_APB2Periph_USART1,
        ENABLE);



    //PA9 TX
    /* 配置PA9为复用推挽输出，用于USART1发送 */

    GPIO_InitStruct.GPIO_Pin=
        GPIO_Pin_9;

    GPIO_InitStruct.GPIO_Mode=
        GPIO_Mode_AF_PP;

    GPIO_InitStruct.GPIO_Speed=
        GPIO_Speed_50MHz;


    GPIO_Init(GPIOA,&GPIO_InitStruct);



    //PA10 RX
    /* 配置PA10为浮空输入，用于USART1接收 */

    GPIO_InitStruct.GPIO_Pin=
        GPIO_Pin_10;

    GPIO_InitStruct.GPIO_Mode=
        GPIO_Mode_IN_FLOATING;


    GPIO_Init(GPIOA,&GPIO_InitStruct);



    /* USART1参数配置：115200, 8N1, 收发模式, 无硬件流控 */
    USART_InitStruct.USART_BaudRate=115200;

    USART_InitStruct.USART_WordLength=
        USART_WordLength_8b;

    USART_InitStruct.USART_StopBits=
        USART_StopBits_1;

    USART_InitStruct.USART_Parity=
        USART_Parity_No;

    USART_InitStruct.USART_Mode=
        USART_Mode_Tx|USART_Mode_Rx;

    USART_InitStruct.USART_HardwareFlowControl=
        USART_HardwareFlowControl_None;


    USART_Init(
        USART1,
        &USART_InitStruct);



    /* NVIC配置：USART1中断，抢占优先级3，子优先级3 */
    NVIC_InitStruct.NVIC_IRQChannel=
        USART1_IRQn;


    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=
        3;


    NVIC_InitStruct.NVIC_IRQChannelSubPriority=
        3;


    NVIC_InitStruct.NVIC_IRQChannelCmd=
        ENABLE;


    NVIC_Init(&NVIC_InitStruct);



    /* 开启USART1接收中断(RXNE：接收数据寄存器非空中断) */
    USART_ITConfig(
        USART1,
        USART_IT_RXNE,
        ENABLE);



    /* 使能USART1外设 */
    USART_Cmd(
        USART1,
        ENABLE);

}




/*
    USART2初始化

    Air780E
    功能：初始化USART2用于与Air780E 4G模块通信
    参数：无
    返回值：无

    配置：
    - 引脚：PA2(TX复用推挽), PA3(RX浮空输入)
    - 波特率：115200
    - 数据位：8位
    - 停止位：1位
    - 校验：无
    - 中断：接收中断(RXNE)，抢占优先级0(最高，确保AT响应不丢失)，子优先级3

    说明：
    USART2挂载在APB1总线上，时钟频率36MHz。
    抢占优先级设为0(最高)，因为Air780E的AT响应不能丢失。
*/


void usart2_init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;



    /* 使能GPIOA时钟(USART2的TX/RX引脚在GPIOA上) */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA,
        ENABLE);



    /* 使能USART2时钟(USART2挂载在APB1总线上) */
    RCC_APB1PeriphClockCmd(
        RCC_APB1Periph_USART2,
        ENABLE);



    //PA2 TX
    /* 配置PA2为复用推挽输出，用于USART2发送 */

    GPIO_InitStruct.GPIO_Pin=
        GPIO_Pin_2;

    GPIO_InitStruct.GPIO_Mode=
        GPIO_Mode_AF_PP;

    GPIO_InitStruct.GPIO_Speed=
        GPIO_Speed_50MHz;


    GPIO_Init(GPIOA,&GPIO_InitStruct);



    //PA3 RX
    /* 配置PA3为浮空输入，用于USART2接收 */


    GPIO_InitStruct.GPIO_Pin=
        GPIO_Pin_3;


    GPIO_InitStruct.GPIO_Mode=
        GPIO_Mode_IN_FLOATING;


    GPIO_Init(GPIOA,&GPIO_InitStruct);



    /* USART2参数配置：115200, 8N1, 收发模式, 无硬件流控 */
    USART_InitStruct.USART_BaudRate=
        115200;


    USART_InitStruct.USART_WordLength=
        USART_WordLength_8b;


    USART_InitStruct.USART_StopBits=
        USART_StopBits_1;


    USART_InitStruct.USART_Parity=
        USART_Parity_No;


    USART_InitStruct.USART_Mode=
        USART_Mode_Tx|USART_Mode_Rx;


    USART_InitStruct.USART_HardwareFlowControl=
        USART_HardwareFlowControl_None;



    USART_Init(
        USART2,
        &USART_InitStruct);



    /* NVIC配置：USART2中断，抢占优先级0(最高)，子优先级3 */
    NVIC_InitStruct.NVIC_IRQChannel=
        USART2_IRQn;


    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=
        0;


    NVIC_InitStruct.NVIC_IRQChannelSubPriority=
        3;


    NVIC_InitStruct.NVIC_IRQChannelCmd=
        ENABLE;



    NVIC_Init(&NVIC_InitStruct);



    /* 开启USART2接收中断(RXNE：接收数据寄存器非空中断) */
    USART_ITConfig(
        USART2,
        USART_IT_RXNE,
        ENABLE);



    /* 使能USART2外设 */
    USART_Cmd(
        USART2,
        ENABLE);

}






/*
    接收缓存
    说明：
    USART1接收缓存和状态变量，用于存储调试串口接收到的数据。
    USART2接收缓存和长度变量，用于存储Air780E的AT响应数据。
*/


u8 USART_RX_BUF[USART_REC_LEN];  /* USART1接收缓存区 */

u16 USART_RX_STA=0;  /* USART1接收状态：bit15=完成,bit14=收到0x0D,bit13~0=计数 */


char USART2_RX_BUF[USART2_REC_LEN];  /* USART2接收缓存区，存储Air780E响应 */

u16 USART2_RX_LEN=0;  /* USART2已接收数据长度 */






/*
    USART1中断
    功能：USART1接收中断处理函数
    参数：无
    返回值：无

    说明：
    每次接收到一个字节触发RXNE中断。
    使用USART_RX_STA状态机协议：
    - 接收到0x0D(回车)：置bit14标志
    - 接着收到0x0A(换行)：置bit15标志，表示接收完成
    - 如果0x0D后不是0x0A：重置状态机
    - 普通字符：存入缓存，计数器递增
    - 缓存满时(达到USART_REC_LEN)：重置计数器
*/


void USART1_IRQHandler(void)
{

    u8 Res;


    /* 检查是否为接收中断(RXNE) */
    if(USART_GetITStatus(
        USART1,
        USART_IT_RXNE)==SET)
    {


        Res=USART_ReceiveData(USART1);  /* 读取接收到的字节数据 */



        /* 检查是否尚未接收完成(bit15=0) */
        if((USART_RX_STA&0x8000)==0)
        {


            /* 检查是否已收到回车0x0D(bit14=1) */
            if(USART_RX_STA&0x4000)
            {


                /* 回车后收到换行0x0A，标记接收完成 */
                if(Res==0x0A)
                {

                    USART_RX_STA|=0x8000;  /* 置bit15，标记接收完成 */

                }

                else
                {

                    USART_RX_STA=0;  /* 回车后不是换行，协议错误，重置状态机 */

                }

            }


            else
            {


                /* 未收到回车，检查当前字符是否为回车0x0D */
                if(Res==0x0D)
                {

                    USART_RX_STA|=0x4000;  /* 置bit14，标记收到回车 */

                }


                else
                {

                    /* 普通字符：存入缓存，计数器递增 */
                    USART_RX_BUF[USART_RX_STA++]=Res;


                    /* 缓存溢出保护：达到最大长度时重置 */
                    if((USART_RX_STA&0x3fff)
                       >=USART_REC_LEN)
                    {

                        USART_RX_STA=0;

                    }

                }


            }


        }


    }

}







/*
    通用发送
    功能：通过指定USART发送字符串
    参数：USARTx - 串口外设(USART1或USART2)
         DataString - 待发送的字符串指针(以'\0'结尾)
    返回值：无

    说明：
    逐字节发送字符串，每个字节发送后等待TXE(发送数据寄存器空)标志。
    全部发送完成后等待TC(发送完成)标志，确保最后字节完全发出。
*/


void USART_SendString(
    USART_TypeDef* USARTx,
    const char *DataString)
{

    while(*DataString)
    {


        USART_SendData(
            USARTx,
            *DataString);  /* 发送当前字节 */



        while(
            USART_GetFlagStatus(
            USARTx,
            USART_FLAG_TXE)
            ==RESET);  /* 等待发送数据寄存器空(TXE) */



        DataString++;  /* 指向下一个字节 */

    }



    while(
        USART_GetFlagStatus(
        USARTx,
        USART_FLAG_TC)
        ==RESET);  /* 等待发送完成(TC)，确保最后字节完全发出 */

}






/*
    USART2发送

    Air780E
    功能：通过USART2发送字符串(主要用于发送AT指令)
    参数：str - 待发送的字符串指针
    返回值：无

    说明：
    封装USART_SendString，固定使用USART2发送。
    MQTT和HTTP共用此函数发送AT指令。
*/


void USART2_SendString(const char *str)
{

    USART_SendString(
        USART2,
        str);

}






/*
    清空USART2缓存
    功能：清空USART2接收缓存区并重置接收长度
    参数：无
    返回值：无

    说明：
    操作顺序：关闭RXNE中断 -> memset清空缓存 -> 重置长度 -> 开启RXNE中断。
    先关闭中断是为了防止清空过程中中断写入新数据导致数据不一致。
*/


void USART2_Clear(void)
{


    /* 关闭USART2接收中断，防止清空过程中中断写入数据 */
    USART_ITConfig(
        USART2,
        USART_IT_RXNE,
        DISABLE);



    /* 使用memset将接收缓存区全部清零 */
    memset(
        USART2_RX_BUF,
        0,
        sizeof(USART2_RX_BUF));



    USART2_RX_LEN=0;  /* 重置接收数据长度为0 */



    /* 重新开启USART2接收中断 */
    USART_ITConfig(
        USART2,
        USART_IT_RXNE,
        ENABLE);


}






/*
    USART2中断

    接收Air780E数据
    功能：USART2接收中断处理函数，接收Air780E返回的AT响应数据
    参数：无
    返回值：无

    说明：
    每次接收到一个字节触发RXNE中断。
    数据存入USART2_RX_BUF缓存，USART2_RX_LEN递增记录长度。
    每次存入后自动在末尾添加'\0'字符串结束符，便于strstr搜索。
    缓存满时重置长度，实现循环覆盖。
    同时将接收到的每个字节转发到USART1，用于PC端实时调试观察。
*/


void USART2_IRQHandler(void)
{


    u8 Res;



    /* 检查是否为接收中断(RXNE) */
    if(USART_GetITStatus(
        USART2,
        USART_IT_RXNE)
        !=RESET)
    {


        Res=
        USART_ReceiveData(USART2);  /* 读取接收到的字节数据 */



        /* 检查缓存是否还有空间(预留1字节给'\0') */
        if(USART2_RX_LEN
            < USART2_REC_LEN-1)
        {


            /* 将字节存入缓存，长度递增 */
            USART2_RX_BUF[USART2_RX_LEN++]
                =Res;



            /* 在当前数据末尾添加字符串结束符，便于strstr搜索 */
            USART2_RX_BUF[USART2_RX_LEN]
                ='\0';


        }

        else
        {

            /* 缓存已满，重置长度，从头开始覆盖写入 */
            USART2_RX_LEN=0;

        }



        //调试输出到USART1
        /* 将接收到的字节同时发送到USART1，便于PC端实时监控Air780E响应 */

        USART_SendData(
            USART1,
            Res);



        while(
            USART_GetFlagStatus(
            USART1,
            USART_FLAG_TXE)
            ==RESET);  /* 等待USART1发送完成 */



    }


}



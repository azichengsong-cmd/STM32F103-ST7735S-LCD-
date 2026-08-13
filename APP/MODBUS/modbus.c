#include "modbus.h"
#include "rs485.h"
#include "usart.h"
#include "delay.h"
#include "sensor.h"


/*================================================
 * Modbus保持寄存器
 *
 * 当前先设置成测试数据
 *
 * 寄存器0 = 100
 * 寄存器1 = 200
 * 寄存器2 = 300
 * ...
 *================================================*/
u16 Modbus_HoldingReg[MODBUS_REG_NUM] =
{
    1,
    200,
    300,
    400,
    500,
    600,
    700,
    800,
    900,
    1000
};


/*================================================
 * Modbus初始化
 *
 * 功能说明：
 *     初始化Modbus RTU从机，将保持寄存器填充为
 *     100、200、300...的测试数据，并通过串口
 *     打印从机配置信息。
 *
 * 参数说明：无
 *
 * 返回值：  无
 *================================================*/
void Modbus_Init(void)
{
    u8 i;

    /*
     * 初始化保持寄存器
     */
    for(i = 0; i < MODBUS_REG_NUM; i++)
    {
        Modbus_HoldingReg[i] = (i + 1) * 100;
    }

    printf("\r\n");
    printf("================================\r\n");
    printf("MODBUS RTU SLAVE START\r\n");
    printf("Slave ID = %d\r\n", MODBUS_SLAVE_ID);
    printf("Function = 03\r\n");
    printf("Register Number = %d\r\n", MODBUS_REG_NUM);
    printf("================================\r\n");
}



/*================================================
 * 更新Modbus保持寄存器
 *
 * 功能说明：
 *     将sensor结构体中的传感器数据（温度、气压、海拔）
 *     经过缩放转换后写入Modbus保持寄存器。
 *     缩放目的是将浮点数转为整数，方便Modbus传输。
 *
 * 参数说明：无
 *
 * 返回值：  无
 *================================================*/
void Modbus_UpdateRegisters(void)
{
    /*
    ==========================================
                寄存器0：温度
    ==========================================

    sensor.temp：

        32.5 ℃

    保存：

        32.5 × 10 = 325

    上位机：

        325 / 10 = 32.5 ℃
    */

    Modbus_HoldingReg[0] =
        (u16)(sensor.temp * 10.0);


    /*
    ==========================================
                寄存器1：气压
    ==========================================

    sensor.pressure：

        101325 Pa

    转换：

        101325 Pa
        = 101.325 kPa

    为了保留两位小数：

        101.325 × 100
        = 10132.5

    转成整数：

        10132

    上位机：

        10132 / 100
        = 101.32 kPa
    */

    Modbus_HoldingReg[1] =
        (u16)(sensor.pressure / 10.0);


    /*
    ==========================================
                寄存器2：海拔
    ==========================================

    sensor.altitude：

        168.4 m

    保存：

        168.4 × 10
        = 1684

    上位机：

        1684 / 10
        = 168.4 m
    */

    Modbus_HoldingReg[2] =
        (u16)(sensor.altitude * 10.0);


    /*
    ==========================================
                预留寄存器
    ==========================================
    */

    Modbus_HoldingReg[3] = 0;
    Modbus_HoldingReg[4] = 0;
    Modbus_HoldingReg[5] = 0;
    Modbus_HoldingReg[6] = 0;
    Modbus_HoldingReg[7] = 0;
    Modbus_HoldingReg[8] = 0;
    Modbus_HoldingReg[9] = 0;
}



/*================================================
 * Modbus CRC16
 *
 * Modbus RTU CRC算法：
 *
 * 初始值：0xFFFF
 * 多项式：0xA001
 *
 * 功能说明：
 *     对指定缓冲区数据计算CRC16校验值，
 *     用于Modbus RTU通信的差错校验。
 *
 * 参数说明：
 *     buf  - 指向待计算CRC的数据缓冲区
 *     len  - 参与计算的数据长度（字节数）
 *
 * 返回值：
 *     计算得到的CRC16值
 *
 *     低8位先发送
 *     高8位后发送
 *================================================*/
u16 Modbus_CRC16(u8 *buf, u16 len)
{
    u16 crc = 0xFFFF;
    u16 i;
    u8 j;

    for(i = 0; i < len; i++)
    {
        crc ^= buf[i];          /* 将当前字节异或到CRC低8位 */

        for(j = 0; j < 8; j++)  /* 逐位处理8个比特 */
        {
            if(crc & 0x0001)    /* 如果最低位为1，右移后异或多项式 */
            {
                crc >>= 1;
                crc ^= 0xA001;  /* 0xA001是0x8005的位反转，即Modbus多项式 */
            }
            else                /* 最低位为0，仅右移 */
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}


/*================================================
 * Modbus异常响应
 *
 * 功能说明：
 *     当从机检测到错误时，向主机发送异常响应帧。
 *     异常响应将功能码最高位置1（0x80），并附带异常码。
 *
 * 正常：
 *
 * 01 03 ...
 *
 * 异常：
 *
 * 01 83 异常码 CRC
 *
 * 03功能码异常 = 0x83
 *
 * 例如：
 *
 * 01 83 02 CRC_L CRC_H
 *
 * 参数说明：
 *     slave_id       - 从机地址
 *     function       - 功能码（原始值，函数内部自动置位0x80）
 *     exception_code - 异常码（如0x02表示地址非法）
 *
 * 返回值：无
 *================================================*/
static void Modbus_SendException(u8 slave_id,
                                 u8 function,
                                 u8 exception_code)
{
    u8 response[5];
    u16 crc;

    response[0] = slave_id;              /* 从机地址 */
    response[1] = function | 0x80;      /* 功能码最高位置1，表示异常响应 */
    response[2] = exception_code;       /* 异常码 */

    crc = Modbus_CRC16(response, 3);    /* 对前3字节计算CRC */

    response[3] = crc & 0xFF;           /* CRC低字节在前 */
    response[4] = (crc >> 8) & 0xFF;   /* CRC高字节在后 */

    RS485_SendData(response, 5);        /* 通过RS485发送异常响应帧 */
}


/*================================================
 * Modbus功能码03
 *
 * 读取保持寄存器
 *
 * 功能说明：
 *     处理主机发来的功能码03请求，校验CRC和地址范围，
 *     从保持寄存器中读取数据并构造响应帧通过RS485发送。
 *
 * 参数说明：
 *     request - 指向接收到的8字节Modbus请求帧
 *
 * 返回值：  无
 *
 * 请求：
 *
 * 01
 * 03
 * 起始地址高
 * 起始地址低
 * 数量高
 * 数量低
 * CRC_L
 * CRC_H
 *
 * 例如：
 *
 * 01 03 00 00 00 0A C5 CD
 *
 * 表示：
 *
 * 从地址0开始
 * 读取10个寄存器
 *================================================*/
static void Modbus_Function03(u8 *request)
{
    u16 start_address;
    u16 quantity;

    u16 crc_receive;
    u16 crc_calculate;

    u8 response[MODBUS_RESPONSE_LEN];

    u16 response_len;

    u16 i;
    u16 value;


    /*================================================
     * 解析起始地址
     *================================================*/
    start_address =
        ((u16)request[2] << 8) |
        request[3];


    /*================================================
     * 解析读取数量
     *================================================*/
    quantity =
        ((u16)request[4] << 8) |
        request[5];


    /*================================================
     * 获取接收到的CRC
     *
     * Modbus：
     *
     * CRC低字节在前
     * CRC高字节在后
     *================================================*/
    crc_receive =
        ((u16)request[7] << 8) |
        request[6];


    /*================================================
     * 计算CRC
     *
     * 只计算前6字节
     *================================================*/
    crc_calculate =
        Modbus_CRC16(request, 6);


    printf("\r\n");
    printf("----- MODBUS REQUEST -----\r\n");

    printf("Slave ID     = %02X\r\n", request[0]);

    printf("Function     = %02X\r\n", request[1]);

    printf("Start Addr   = %04X\r\n", start_address);

    printf("Quantity     = %d\r\n", quantity);

    printf("CRC Receive  = %04X\r\n", crc_receive);

    printf("CRC Calculate= %04X\r\n", crc_calculate);


    /*================================================
     * CRC错误
     *================================================*/
    if(crc_receive != crc_calculate)
    {
        printf("CRC ERROR!\r\n");

        return;
    }


    printf("CRC OK\r\n");


    /*================================================
     * 判断地址范围
     *================================================*/

    if(start_address >= MODBUS_REG_NUM)
    {
        printf("ADDRESS ERROR!\r\n");

        Modbus_SendException(
            request[0],
            request[1],
            0x02
        );

        return;
    }


    /*================================================
     * 判断读取数量
     *
     * 03功能码一次最多读取125个
     *
     * 我们当前只有10个寄存器
     *================================================*/

    if(quantity == 0 ||
       quantity > MODBUS_REG_NUM ||
       (start_address + quantity) > MODBUS_REG_NUM)
    {
        printf("QUANTITY ERROR!\r\n");

        Modbus_SendException(
            request[0],
            request[1],
            0x02
        );

        return;
    }


    /*================================================
     * 构造响应
     *
     * 响应格式：
     *
     * ID
     * 功能码
     * 字节数
     * 数据
     * CRC
     *================================================*/

    response[0] = request[0];           /* 从机地址，原样回填 */

    response[1] = 0x03;                 /* 功能码03 */

    response[2] = quantity * 2;         /* 字节数 = 寄存器数 × 2（每个寄存器2字节） */


    /*================================================
     * 填充寄存器数据
     *
     * Modbus寄存器：
     *
     * 高字节
     * 低字节
     *================================================*/

    for(i = 0; i < quantity; i++)
    {
        value =
            Modbus_HoldingReg[start_address + i];  /* 从保持寄存器数组读取数据 */

        response[3 + i * 2] =
            (value >> 8) & 0xFF;                   /* 高字节在前 */

        response[4 + i * 2] =
            value & 0xFF;                          /* 低字节在后 */
    }


    /*================================================
     * 响应帧长度
     *
     * 3个头部字节
     * +
     * quantity * 2
     * +
     * 2个CRC
     *================================================*/

    response_len =
        3 +
        quantity * 2 +
        2;


    /*================================================
     * 计算CRC
     *================================================*/

    crc_calculate =
        Modbus_CRC16(
            response,
            response_len - 2
        );


    /*================================================
     * CRC低字节
     *================================================*/

    response[response_len - 2] =
        crc_calculate & 0xFF;


    /*================================================
     * CRC高字节
     *================================================*/

    response[response_len - 1] =
        (crc_calculate >> 8) & 0xFF;


    /*================================================
     * 打印响应
     *================================================*/

    printf("----- MODBUS RESPONSE -----\r\n");

    printf("TX:");

    for(i = 0; i < response_len; i++)
    {
        printf(
            " %02X",
            response[i]
        );
    }

    printf("\r\n");


    /*================================================
     * RS485发送响应
     *================================================*/

    RS485_SendData(
        response,
        response_len
    );


    printf("RESPONSE SENT\r\n");
}


/*================================================
 * Modbus从机任务
 *
 * 功能说明：
 *     非阻塞式处理Modbus从机任务。从RS485接收缓冲区中
 *     查找合法的Modbus请求帧，找到后处理功能码03请求
 *     并发送响应。处理过程中临时关闭接收中断以防止
 *     缓冲区被并发修改。
 *
 * 参数说明：无
 *
 * 返回值：  无
 *
 * 当前第一阶段：
 *
 * 固定接收8字节
 *
 * 因为我们已经验证电脑每次发送：
 *
 * 01 03 00 00 00 0A C5 CD
 *
 * 共8字节
 *
 * 后续真正完善协议栈时，
 * 再改成基于接收中断/帧间隔的方式。
 *================================================*/
void Modbus_Process(void)
{
    u8 request[MODBUS_REQUEST_LEN];

    u16 rx_len;
    u16 i;

    /*
    ==========================================
            关键：
            先关闭UART4接收中断
    ==========================================
    
    防止：
    
        UART4中断
            ↓
        修改RS485_RX_LEN
            ↓
        主循环同时读取RS485_RX_LEN
    
    */

    USART_ITConfig(
        UART4,
        USART_IT_RXNE,
        DISABLE
    );


    /*
    ==========================================
            获取当前已经收到的数据长度
    ==========================================
    */

    rx_len = RS485_RX_LEN;


    /*
    ==========================================
            没有数据
    ==========================================
    */

    if(rx_len < MODBUS_REQUEST_LEN)
    {
        USART_ITConfig(
            UART4,
            USART_IT_RXNE,
            ENABLE
        );

        return;
    }


    /*
    ==========================================
            在缓冲区寻找合法Modbus帧
    ==========================================
    */

    for(i = 0;
        i <= rx_len - MODBUS_REQUEST_LEN;
        i++)
    {
        /*
        Slave ID
        */

        if(RS485_RX_BUF[i] != MODBUS_SLAVE_ID)
        {
            continue;
        }


        /*
        Function 03
        */

        if(RS485_RX_BUF[i + 1] != 0x03)
        {
            continue;
        }


        /*
        ======================================
                找到8字节请求
        ======================================
        */

        request[0] = RS485_RX_BUF[i + 0];
        request[1] = RS485_RX_BUF[i + 1];
        request[2] = RS485_RX_BUF[i + 2];
        request[3] = RS485_RX_BUF[i + 3];
        request[4] = RS485_RX_BUF[i + 4];
        request[5] = RS485_RX_BUF[i + 5];
        request[6] = RS485_RX_BUF[i + 6];
        request[7] = RS485_RX_BUF[i + 7];


        /*
        ======================================
                CRC
        ======================================
        */

        {
            u16 crc_receive;
            u16 crc_calculate;

            crc_receive =
                ((u16)request[7] << 8)
                |
                request[6];

            crc_calculate =
                Modbus_CRC16(
                    request,
                    6
                );


            /*
            ==================================
                    CRC正确
            ==================================
            */

            if(crc_receive == crc_calculate)
            {
                /*
                ==================================
                        先把已经找到的帧
                        从接收缓冲区删除
                ==================================
                */

                {
                    u16 remain;
                    u16 j;

                    remain =
                        rx_len -
                        (
                            i +
                            MODBUS_REQUEST_LEN
                        );


                    /*
                    后面的数据向前移动
                    */

                    for(j = 0;
                        j < remain;
                        j++)
                    {
                        RS485_RX_BUF[j] =
                            RS485_RX_BUF[
                                i +
                                MODBUS_REQUEST_LEN +
                                j
                            ];
                    }


                    RS485_RX_LEN = remain;
                }


                /*
                ==================================
                        恢复UART4接收中断
                ==================================
                */

                USART_ITConfig(
                    UART4,
                    USART_IT_RXNE,
                    ENABLE
                );


                /*
                ==================================
                        打印收到的数据
                ==================================
                */

                printf("\r\n");
                printf("RX:");

                for(u8 j = 0;
                    j < MODBUS_REQUEST_LEN;
                    j++)
                {
                    printf(
                        " %02X",
                        request[j]
                    );
                }

                printf("\r\n");


                printf(
                    "CRC Receive  = %04X\r\n",
                    crc_receive
                );

                printf(
                    "CRC Calculate= %04X\r\n",
                    crc_calculate
                );

                printf(
                    "CRC OK\r\n"
                );


                /*
                ==================================
                        处理Function 03
                ==================================
                */

                Modbus_Function03(
                    request
                );


                /*
                ==================================
                        一帧处理完成
                ==================================
                */

                return;
            }
        }
    }


    /*
    ==========================================
            没找到合法帧
    ==========================================
    
    只保留最后一个字节
    */

    if(rx_len > 0)
    {
        RS485_RX_BUF[0] =
            RS485_RX_BUF[rx_len - 1];

        RS485_RX_LEN = 1;
    }


    /*
    ==========================================
            恢复UART4接收中断
    ==========================================
    */

    USART_ITConfig(
        UART4,
        USART_IT_RXNE,
        ENABLE
    );
}



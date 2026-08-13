#include "bmp180.h"
#include "iic.h"
#include "delay.h"
#include "usart.h"
#include "math.h"



BMP180_TypeDef bmp180;



/*================================================
 * BMP180读取一个字节
 *
 * 功能说明：
 *     通过IIC总线从BMP180指定寄存器读取一个字节数据。
 *     发送写操作设定寄存器地址后，重新发起读操作读取数据。
 *
 * 参数说明：
 *     reg - 要读取的寄存器地址
 *
 * 返回值：
 *     读取到的字节数据
 *================================================*/
u8 BMP180_ReadByte(u8 reg)
{

u8 data;


IIC_Start();                    /* 发起IIC起始信号 */


IIC_Send_Byte(BMP180_ADDR<<1);  /* 发送BMP180设备地址 + 写方向（最低位0） */

IIC_Wait_Ack();                 /* 等待BMP180应答 */



IIC_Send_Byte(reg);             /* 发送要读取的寄存器地址 */

IIC_Wait_Ack();                 /* 等待应答 */



IIC_Start();                    /* 重新发起起始信号（Repeated Start） */


IIC_Send_Byte((BMP180_ADDR<<1)|1); /* 发送BMP180设备地址 + 读方向（最低位1） */

IIC_Wait_Ack();                 /* 等待应答 */



data=IIC_Read_Byte(0);          /* 读取一个字节，发送NAK（不再继续读） */



IIC_Stop();                     /* 发送IIC停止信号 */


return data;

}






/*================================================
 * BMP180写入一个字节
 *
 * 功能说明：
 *     通过IIC总线向BMP180指定寄存器写入一个字节数据。
 *
 * 参数说明：
 *     reg  - 要写入的寄存器地址
 *     data - 要写入的数据
 *
 * 返回值：无
 *================================================*/
void BMP180_WriteByte(u8 reg,u8 data)
{


IIC_Start();                    /* 发起IIC起始信号 */


IIC_Send_Byte(BMP180_ADDR<<1);  /* 发送BMP180设备地址 + 写方向 */

IIC_Wait_Ack();                 /* 等待应答 */



IIC_Send_Byte(reg);             /* 发送寄存器地址 */

IIC_Wait_Ack();                 /* 等待应答 */



IIC_Send_Byte(data);            /* 发送要写入的数据 */

IIC_Wait_Ack();                 /* 等待应答 */



IIC_Stop();                     /* 发送IIC停止信号 */


}



/*================================================
 * BMP180读取16bit数据
 *
 * 功能说明：
 *     通过IIC总线从BMP180连续读取2个字节，组合成
 *     16位有符号整数。主要用于读取校准参数。
 *     BMP180的16位数据采用大端格式（高字节在前）。
 *
 * 参数说明：
 *     reg - 起始寄存器地址
 *
 * 返回值：
 *     读取到的16位有符号数据
 *
 * 用于校准参数
 *================================================*/
short BMP180_Read16(u8 reg)
{

short data;



IIC_Start();                    /* 发起IIC起始信号 */


IIC_Send_Byte(BMP180_ADDR<<1);  /* 发送设备地址 + 写方向 */

IIC_Wait_Ack();                 /* 等待应答 */



IIC_Send_Byte(reg);             /* 发送起始寄存器地址 */

IIC_Wait_Ack();                 /* 等待应答 */



IIC_Start();                    /* 重新发起起始信号（Repeated Start） */


IIC_Send_Byte((BMP180_ADDR<<1)|1); /* 发送设备地址 + 读方向 */

IIC_Wait_Ack();                 /* 等待应答 */



data=IIC_Read_Byte(1);          /* 读取高字节，发送ACK表示继续读 */


data <<=8;                      /* 高字节左移8位 */


data|=IIC_Read_Byte(0);         /* 读取低字节，发送NAK表示读取结束 */



IIC_Stop();                     /* 发送IIC停止信号 */



return data;

}





/*================================================
 * BMP180读取芯片ID
 *
 * 功能说明：
 *     读取BMP180的芯片ID寄存器（0xD0），
 *     正常返回值应为0x55，用于验证设备通信正常。
 *
 * 参数说明：无
 *
 * 返回值：
 *     芯片ID（正常为0x55）
 *================================================*/

u8 BMP180_ReadID(void)
{

return BMP180_ReadByte(BMP180_CHIP_ID_REG); /* 读取芯片ID寄存器 */

}





/*================================================
 * BMP180读取校准参数
 *
 * 功能说明：
 *     从BMP180的校准寄存器（0xAA~0xBE）读取11个
 *     校准参数（AC1~AC6, B1, B2, MB, MC, MD），
 *     存储到bmp180结构体中，用于后续温压补偿计算。
 *     这些参数是每个芯片出厂时 individually 校准的。
 *
 * 参数说明：无
 *
 * 返回值：无
 *================================================*/

void BMP180_ReadCalibration(void)
{


bmp180.AC1=BMP180_Read16(0xAA);    /* AC1校准参数 */

bmp180.AC2=BMP180_Read16(0xAC);    /* AC2校准参数 */

bmp180.AC3=BMP180_Read16(0xAE);    /* AC3校准参数 */



bmp180.AC4=(unsigned short)BMP180_Read16(0xB0); /* AC4校准参数（无符号） */

bmp180.AC5=(unsigned short)BMP180_Read16(0xB2); /* AC5校准参数（无符号） */

bmp180.AC6=(unsigned short)BMP180_Read16(0xB4); /* AC6校准参数（无符号） */



bmp180.B1=BMP180_Read16(0xB6);     /* B1校准参数 */

bmp180.B2=BMP180_Read16(0xB8);     /* B2校准参数 */



bmp180.MB=BMP180_Read16(0xBA);     /* MB校准参数 */

bmp180.MC=BMP180_Read16(0xBC);     /* MC校准参数 */

bmp180.MD=BMP180_Read16(0xBE);     /* MD校准参数 */


}





/*================================================
 * BMP180初始化
 *
 * 功能说明：
 *     延时等待BMP180上电稳定，读取芯片ID验证通信，
 *     ID正确（0x55）则读取校准参数并打印状态。
 *
 * 参数说明：无
 *
 * 返回值：无
 *================================================*/

void BMP180_Init(void)
{


u8 id;


delay_ms(10);                     /* 等待BMP180上电稳定 */



id=BMP180_ReadID();               /* 读取芯片ID */



printf("BMP180 ID=%d\r\n",id);    /* 打印芯片ID */



if(id==0x55)                      /* ID为0x55表示通信正常 */
{


printf("BMP180 OK\r\n");         /* 打印通信正常信息 */



BMP180_ReadCalibration();        /* 读取校准参数 */



printf("Calibration OK\r\n");    /* 打印校准完成信息 */



}
else
{


printf("BMP180 ERROR\r\n");      /* 打印通信错误信息 */


}


}







/*================================================
 * BMP180读取温度
 *
 * 功能说明：
 *     发送温度测量命令，等待转换完成后读取原始温度数据，
 *     利用校准参数按照BMP180 datasheet公式计算真实温度。
 *
 * 参数说明：无
 *
 * 返回值:
 *     温度值，单位0.1℃
 *     例如: 305 表示 30.5℃
 *================================================*/

long BMP180_ReadTemperature(void)
{


long X1,X2;



BMP180_WriteByte(
BMP180_CONTROL,
BMP180_TEMP_CMD
);                                /* 写入温度测量命令到控制寄存器 */



delay_ms(5);                      /* 等待温度转换完成（最大4.5ms） */



bmp180.UT=BMP180_Read16(BMP180_RESULT); /* 读取未补偿的原始温度值 */



X1=((bmp180.UT-bmp180.AC6)*bmp180.AC5)>>15; /* X1 = (UT - AC6) * AC5 / 2^15 */



X2=((long)bmp180.MC<<11)/(X1+bmp180.MD);     /* X2 = MC * 2^11 / (X1 + MD) */



bmp180.B5=X1+X2;                            /* B5 = X1 + X2，用于压力计算 */



bmp180.temperature=(bmp180.B5+8)>>4;        /* 真实温度 = (B5 + 8) / 2^4，单位0.1℃ */



return bmp180.temperature;


}







/*================================================
 * BMP180读取原始压力数据
 *
 * 功能说明：
 *     直接从BMP180的F6/F7/F8寄存器读取3字节（19bit）
 *     原始压力数据。OSS=0时需右移8位对齐。
 *     注意：此函数不发送测量命令，需先调用温度测量
 *     以获得B5值，再由BMP180_ReadPressure统一处理。
 *
 * 参数说明：无
 *
 * 返回值：
 *     原始压力数据（19bit，已右移8位对齐）
 *================================================*/

long BMP180_ReadUP(void)
{

long UP;



IIC_Start();                    /* 发起IIC起始信号 */



IIC_Send_Byte(BMP180_ADDR<<1);  /* 发送设备地址 + 写方向 */

IIC_Wait_Ack();                 /* 等待应答 */



IIC_Send_Byte(BMP180_RESULT);   /* 发送结果寄存器地址 */

IIC_Wait_Ack();                 /* 等待应答 */



IIC_Start();                    /* 重新发起起始信号 */



IIC_Send_Byte((BMP180_ADDR<<1)|1); /* 发送设备地址 + 读方向 */

IIC_Wait_Ack();                 /* 等待应答 */



UP=IIC_Read_Byte(1);            /* 读取第1字节（MSB），发送ACK */


UP<<=8;                         /* 左移8位 */


UP|=IIC_Read_Byte(1);           /* 读取第2字节，发送ACK */


UP<<=8;                         /* 再左移8位 */


UP|=IIC_Read_Byte(0);           /* 读取第3字节（LSB），发送NAK */



IIC_Stop();                     /* 发送IIC停止信号 */



UP>>=8;                         /* OSS=0时右移8位对齐 */



return UP;

}








/*================================================
 * BMP180读取气压
 *
 * 功能说明：
 *     发送压力测量命令，读取原始压力数据，
 *     利用校准参数和温度补偿值B5按照datasheet公式
 *     进行多步补偿计算，得到真实气压值。
 *     注意：调用前必须先调用BMP180_ReadTemperature()
 *     以获得B5值。
 *
 * 参数说明：无
 *
 * 返回值：
 *     气压值，单位Pa
 *     例如：101325 表示标准大气压
 *================================================*/

long BMP180_ReadPressure(void)
{

    long X1;
    long X2;
    long X3;

    long B3;
    long B6;

    unsigned long B4;
    unsigned long B7;

    unsigned long UP;



    /*
        开始压力转换
        OSS=0（过采样率0，精度最低但速度最快）
    */

    BMP180_WriteByte(
        BMP180_CONTROL,
        BMP180_PRESS_CMD
    );                              /* 写入压力测量命令（OSS=0） */


    delay_ms(5);                    /* 等待压力转换完成 */



    /*
        读取F6 F7 F8 三字节原始压力数据
    */


    IIC_Start();


    IIC_Send_Byte(BMP180_ADDR<<1);

    IIC_Wait_Ack();


    IIC_Send_Byte(BMP180_RESULT);

    IIC_Wait_Ack();



    IIC_Start();


    IIC_Send_Byte((BMP180_ADDR<<1)|1);

    IIC_Wait_Ack();



    UP = IIC_Read_Byte(1);          /* 读取高字节 */


    UP <<=8;                        /* 左移8位 */


    UP |= IIC_Read_Byte(1);         /* 读取中字节 */



    UP <<=8;                        /* 再左移8位 */


    UP |= IIC_Read_Byte(0);         /* 读取低字节，发送NAK */



    IIC_Stop();



    /*
        OSS=0
        右移8位对齐
    */

    UP >>=8;                        /* OSS=0时右移8位 */


    bmp180.UP=UP;                   /* 保存原始压力值 */



    /*
        压力补偿计算
        以下公式来自BMP180 datasheet
    */


    B6=bmp180.B5-4000;              /* B6 = B5 - 4000，用于计算补偿值 */



    X1=(bmp180.B2*((B6*B6)>>12))>>11; /* X1 = B2 * (B6^2 / 2^12) / 2^11 */


    X2=(bmp180.AC2*B6)>>11;           /* X2 = AC2 * B6 / 2^11 */


    X3=X1+X2;                         /* X3 = X1 + X2 */



    B3=((((long)bmp180.AC1*4+X3)+2)>>2); /* B3 = (((AC1*4 + X3) + 2) / 2^2 */



    X1=(bmp180.AC3*B6)>>13;          /* X1 = AC3 * B6 / 2^13 */


    X2=(bmp180.B1*((B6*B6)>>12))>>16; /* X2 = B1 * (B6^2 / 2^12) / 2^16 */


    X3=((X1+X2)+2)>>2;               /* X3 = ((X1 + X2) + 2) / 2^2 */



    B4=((unsigned long)bmp180.AC4*
       (unsigned long)(X3+32768))>>15; /* B4 = AC4 * (X3 + 32768) / 2^15 */



    B7=((unsigned long)UP-B3)*50000;   /* B7 = (UP - B3) * 50000 */



    if(B7 < 0x80000000)               /* 根据B7的最高位选择不同的计算方式避免溢出 */
    {

        bmp180.pressure=(B7<<1)/B4;   /* B7 < 2^31 时：pressure = (B7 * 2) / B4 */

    }
    else
    {

        bmp180.pressure=(B7/B4)<<1;   /* B7 >= 2^31 时：pressure = (B7 / B4) * 2 */

    }




    /*
        二次修正（精细补偿）
    */


    X1=(bmp180.pressure>>8)*
       (bmp180.pressure>>8);          /* X1 = (pressure / 2^8)^2 */


    X1=(X1*3038)>>16;                 /* X1 = X1 * 3038 / 2^16 */


    X2=(-7357*bmp180.pressure)>>16;   /* X2 = -7357 * pressure / 2^16 */


    bmp180.pressure +=
        ((X1+X2+3791)>>4);            /* pressure += (X1 + X2 + 3791) / 2^4 */



    return bmp180.pressure;


}





/*================================================
 * BMP180计算海拔
 *
 * 功能说明：
 *     根据气压值和标准海平面气压（101325Pa）计算海拔高度。
 *     使用国际标准大气模型公式：
 *     altitude = 44330 * (1 - (P/P0)^0.1903)
 *
 * 参数说明：无
 *
 * 返回值：
 *     海拔高度，单位米（m）
 *     注意：调用前必须先调用BMP180_ReadPressure()
 *================================================*/

long BMP180_ReadAltitude(void)
{


float altitude;



altitude=
44330*
(
1-
pow(
((float)bmp180.pressure/101325), /* 实际气压与标准海平面气压的比值 */
0.1903                            /* 气压-高度指数系数 */
)
);



bmp180.altitude=(long)altitude;    /* 转为长整型保存 */



return bmp180.altitude;


}



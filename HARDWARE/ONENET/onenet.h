/*
==================================================
    OneNET云平台驱动头文件 (onenet.h)

    功能：
    声明OneNET MQTT连接和数据上传相关函数。
    通过USART2与Air780E 4G模块通信，
    使用AT指令实现MQTT协议连接OneNET云平台。

    云平台信息：
    - 服务器：mqtts.heclouds.com:1883
    - 协议：MQTT
    - 数据格式：OneJSON物模型属性上报

    上传数据：
    - 温度(temp)
    - 气压(pressure)
    - 海拔(altitude)
==================================================
*/
#ifndef __ONENET_H
#define __ONENET_H

#include "sys.h"


/*
    OneNET连接初始化
    功能：通过AT指令完成Air780E的MQTT连接流程
    参数：无
    返回值：0=连接成功，1~7=不同步骤失败(详见代码注释)
*/
u8 Connect_init(void);


/*
    上传温度数据
    功能：通过MQTT发布消息将温度数据上传到OneNET
    参数：temp - 温度值(摄氏度，浮点数)
    返回值：0=上传成功，1=上传失败
*/
u8 Up_Temp_Data(double temp);


/*
    上传气压数据
    功能：通过MQTT发布消息将气压数据上传到OneNET
    参数：pressure - 气压值(hPa，浮点数)
    返回值：0=上传成功，1=上传失败
*/
u8 Up_Pressure_Data(double pressure);


/*
    上传海拔数据
    功能：通过MQTT发布消息将海拔数据上传到OneNET
    参数：altitude - 海拔值(米，浮点数)
    返回值：0=上传成功，1=上传失败
*/
u8 Up_Altitude_Data(double altitude);


#endif



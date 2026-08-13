#ifndef __SENSOR_H
#define __SENSOR_H

#include "sys.h"

/*
==================================================
                传感器数据模块
==================================================
功能：
    封装 BMP180 气压传感器的数据采集
    提供温度、气压、海拔三项环境数据
==================================================
*/

/*
    传感器数据结构体

    temp     : 温度（单位：摄氏度）
    pressure : 气压（单位：帕斯卡 Pa）
    altitude : 海拔（单位：米 m，由气压换算得出）
*/
typedef struct
{

double temp;        /* 温度（摄氏度） */

double  pressure;   /* 气压（Pa） */

double altitude;    /* 海拔（米） */


}Sensor_Data;


/* 全局传感器数据实例，供其他模块读取 */
extern Sensor_Data sensor;

/*
    传感器初始化
    将温度、气压、海拔清零
*/
void Sensor_Init(void);

/*
    传感器数据更新
    从 BMP180 读取温度和气压，并计算海拔
    应在主循环中周期性调用
*/
void Sensor_Update(void);

#endif




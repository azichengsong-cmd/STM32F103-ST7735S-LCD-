#ifndef __ALARM_H
#define __ALARM_H

#include "sys.h"


/*
==================================================
                报警模块
==================================================
功能：
    监测温度和气压是否超过阈值
    超限时点亮 LED 进行报警提示
==================================================
*/

/* 默认阈值（已废弃，改为使用 max 模块的动态阈值） */

//#define TEMP_MAX 20

//#define PRESS_MAX 101325


/*
    报警初始化
    初始化 LED 状态（默认熄灭）
*/
void Alarm_Init(void);


/*
    报警检测
    比较当前温度和气压与设定阈值，
    超限则点亮对应 LED

    参数：
        temp     当前温度（未使用，直接读取全局 sensor）
        pressure 当前气压（未使用，直接读取全局 sensor）
*/
void Alarm_Check(float temp,long pressure);



#endif


#ifndef __WEATHER_H
#define __WEATHER_H

#include "sys.h"

/*
==================================================
                天气数据模块
==================================================
功能：
    通过 Air780E 4G 模块连接心知天气 API
    获取指定城市的实时天气信息
==================================================
*/

/*
    天气数据结构体

    city        : 城市名称（英文，如 "Zhengzhou"）
    text        : 天气描述（英文，如 "Sunny"、"Cloudy"）
    temperature : 室外温度字符串（如 "25"）
*/
typedef struct
{
    char city[32];          /* 城市名称 */
    char text[32];          /* 天气描述文本 */
    char temperature[8];    /* 室外温度 */
} WeatherData;

/* 全局天气数据实例，供 UI 模块读取显示 */
extern WeatherData weather;

/*
    获取天气数据

    通过 Air780E 模块发起 HTTP GET 请求
    从心知天气 API 获取郑州的实时天气
    解析 JSON 响应并填充全局 weather 结构体

    返回值：
        0 : 成功获取并解析天气数据
        1 : HTTP ACTION 请求失败
        2 : 响应中未找到 JSON 数据
        3 : JSON 解析失败
*/
u8 Weather_Get(void);

#endif


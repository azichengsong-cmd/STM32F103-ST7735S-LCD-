#ifndef __UI_H
#define __UI_H

#include "sys.h"

/*
==================================================
                用户界面（UI）模块
==================================================
功能：
    管理 LCD 屏幕上的三个页面：主界面、设置界面、历史界面
    处理按键输入进行页面切换和参数调整
    显示传感器数据、天气信息和历史平均值
==================================================
*/




/*
    页面状态枚举

    定义当前显示的界面状态，
    通过 WK_UP 按键在三个界面之间循环切换
*/
typedef enum
{

    UI_MAIN = 0,    /* 主界面：显示传感器数据和天气 */
    UI_SETTING,     /* 设置界面：调整温度和气压阈值 */
    UI_HISTORY      /* 历史数据界面：显示历史平均值 */


}UI_STATE;


/*
    设置页面当前修改项枚举

    在设置界面中通过 DOWN 按键切换，
    决定当前正在调整温度阈值还是气压阈值
*/

typedef enum
{

    SET_TEMP_MAX = 0,     /* 温度阈值 */

    SET_PRESS_MAX         /* 气压阈值 */


}SETTING_ITEM;


/*
    UI 初始化
    设置默认界面状态和刷新标志
*/
void UI_Init(void);

/*
    UI 显示管理
    根据当前界面状态调用对应的绘制函数
    仅在 ui_refresh 为 1 时刷新
*/
void UI_Show(void);

/*
    UI 按键处理
    扫描按键并执行页面切换、参数调整等操作
*/
void UI_Key_Handler(void);


/*
    主界面绘制
    显示标题、天气信息、传感器数据
*/
void UI_Main(void);

/*
    设置界面绘制
    显示温度和气压阈值，带选择箭头
*/
void UI_Setting(void);

/*
    历史数据界面绘制
    显示历史平均温度、气压、海拔和采样次数
*/
void UI_History(void);

/*
    根据英文天气文本显示中文天气
    将 API 返回的英文天气描述映射为中文显示

    参数：
        x    : 显示起始 X 坐标
        y    : 显示起始 Y 坐标
        text : 英文天气文本指针（如 "Sunny"、"Cloudy"）
*/
void UI_ShowWeatherText(u16 x, u16 y, char *text);

/*
    只刷新主页面的传感器数据
    局部更新温度、气压、海拔数值
    不重绘整个界面，避免闪烁
*/
void UI_ShowSensor(void);


#endif



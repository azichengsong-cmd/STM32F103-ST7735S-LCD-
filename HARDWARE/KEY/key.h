/*
==================================================
    按键驱动头文件 (key.h)

    功能：
    定义4个按键的GPIO引脚宏和按键扫描返回值宏。
    提供按键初始化和按键扫描函数声明。

    硬件连接：
    - DOWN  : PC4 (上拉输入，按下为低电平)
    - LEFT  : PC1 (上拉输入，按下为低电平)
    - RIGHT : PC5 (上拉输入，按下为低电平)
    - WKUP  : PA0 (下拉输入，按下为高电平)
==================================================
*/
#ifndef __KEY_H
#define __KEY_H
#include "sys.h"



/* 按键扫描返回值宏定义，Key_scan()返回这些值来区分哪个按键被按下 */
#define DOWN_PRES   1   /* DOWN键按下返回值 */
#define LEFT_PRES   2   /* LEFT键按下返回值 */
#define RIGHT_PRES  3   /* RIGHT键按下返回值 */
#define WKUP_PRES   4   /* WKUP键按下返回值 */



/*
    按键GPIO读取宏定义
    通过GPIO_ReadInputDataBit读取对应引脚的电平状态
    DOWN/LEFT/RIGHT为上拉输入：未按下=1，按下=0
    WKUP为下拉输入：未按下=0，按下=1
*/
#define DOWN  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4)  /* 读取PC4电平 (DOWN键) */
#define LEFT  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1)  /* 读取PC1电平 (LEFT键) */
#define RIGHT GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)  /* 读取PC5电平 (RIGHT键) */
#define WKUP  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)  /* 读取PA0电平 (WKUP键) */

/*
    按键初始化函数
    配置4个按键对应的GPIO引脚为输入模式
*/
void key_init(void);

/*
    按键扫描函数
    参数：mode - 0:不支持连按(按一次返回一次), 1:支持连按(长按持续返回)
    返回值：0=无按键按下，1=DOWN，2=LEFT，3=RIGHT，4=WKUP
*/
u8 Key_scan(u8 mode);
#endif



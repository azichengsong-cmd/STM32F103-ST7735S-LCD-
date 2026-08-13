#ifndef __MAX_H
#define __MAX_H

#include "sys.h"


/*
==================================================
                阈值配置模块
==================================================
功能：
    定义温度和气压的上下限阈值
    供报警模块和设置界面使用
==================================================
*/

/*
    阈值结构体

    temp_max      : 温度上限（摄氏度），超过此值触发温度报警
    pressure_max  : 气压上限（Pa），超过此值触发气压报警
    pressure_min  : 气压下限（Pa），低于此值触发气压报警
*/
typedef struct
{

    double temp_max;        /* 温度上限 */

    double  pressure_max;     /* 气压上限 */

    double pressure_min;     /* 气压下限 */


}MAX_TypeDef;


/* 全局阈值实例，在 max.c 中初始化默认值 */
extern MAX_TypeDef max;



#endif



#include "alarm.h"
#include "led.h"
#include "beep.h"
#include "max.h"
#include "sensor.h"


/*
==================================================
    函数功能：报警模块初始化
    参数说明：无
    返回值  ：无
    说明    ：LED1 初始化为高电平（熄灭），
              LED 低电平点亮，高电平熄灭
==================================================
*/
void Alarm_Init(void)
{

    /* LED1 置高，默认熄灭状态 */
    LED1 = 1;


}



/*
==================================================
    函数功能：报警检测
    参数说明：
        temp     当前温度（实际未使用，直接读取全局 sensor.temp）
        pressure 当前气压（实际未使用，直接读取全局 sensor.pressure）
    返回值  ：无
    说明    ：温度超限点亮 LED1，气压超限点亮 LED2
              LED 低电平点亮，高电平熄灭
==================================================
*/
void Alarm_Check(float temp,long pressure)
{


    /* 温度超过上限，点亮 LED1 报警 */
    if(sensor.temp > max.temp_max)
    {

			
    LED1 = 0;   /* 低电平，LED1 点亮 */

 

    }
    else
    {

         LED1 = 1;   /* 高电平，LED1 熄灭 */

    

    }



    /* 气压超过上限，点亮 LED2 报警 */
    if(sensor.pressure > max.pressure_max)
    {

         LED2 = 0;   /* 低电平，LED2 点亮 */

    

    }else{
		
			LED2 = 1;   /* 高电平，LED2 熄灭 */
			
		}


}


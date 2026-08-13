#include "sensor.h"

#include "bmp180.h"
#include "math.h"

/* 全局传感器数据实例 */
Sensor_Data sensor;

/*
==================================================
    函数功能：传感器初始化
    参数说明：无
    返回值  ：无
    说明    ：将温度、气压、海拔全部清零，
              在系统启动时调用
==================================================
*/
void Sensor_Init(void)
{

sensor.temp=0;          /* 温度清零 */

sensor.pressure=0;      /* 气压清零 */

sensor.altitude=0;      /* 海拔清零 */


}

/*
==================================================
    函数功能：传感器数据更新
    参数说明：无
    返回值  ：无
    说明    ：从 BMP180 读取温度和气压，
              再根据标准大气压公式计算海拔
==================================================
*/
void Sensor_Update(void)
{

/* BMP180_ReadTemperature 返回值为 0.1 度，需除以 10.0 转换为摄氏度 */
sensor.temp = BMP180_ReadTemperature()/10.0;

/* 读取气压值，单位为 Pa */
sensor.pressure = BMP180_ReadPressure();

/*
    海拔计算公式（国际标准大气压公式）：
    altitude = 44330 * (1 - (P/P0)^0.1903)
    其中 P0 = 101325 Pa（标准海平面气压）
    该公式利用气压与海拔的关系进行换算
*/
sensor.altitude = 44330*(1-pow(((float)sensor.pressure/101325),0.1903));


}



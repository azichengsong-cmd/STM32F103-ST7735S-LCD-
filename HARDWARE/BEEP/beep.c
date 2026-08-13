/*
==================================================
    蜂鸣器驱动源文件 (beep.c)

    功能：
    实现蜂鸣器的GPIO初始化配置。
    使用PC9引脚控制有源蜂鸣器。

    硬件连接：
    - BEEP : PC9 (推挽输出，高电平鸣响)

    说明：
    蜂鸣器为高电平鸣响，初始化时输出低电平使其静音。
==================================================
*/
#include "beep.h"

/*
    函数名：beep_init
    功能：初始化蜂鸣器对应的GPIO引脚
    参数：无
    返回值：无

    说明：
    配置PC9为推挽输出模式，速度50MHz。
    初始化后输出低电平，使蜂鸣器处于静音状态。
    后续通过BEEP宏(=1鸣响, =0静音)控制蜂鸣器。
*/
void beep_init(){
	GPIO_InitTypeDef GPIO_InitStruct;/*通过结构体类型定义结构体变量*/

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);/*使能GPIOC的端口时钟*/

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;/*PIN引脚:PC9*/
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;/*工作模式：推挽输出*/
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;/*工作速度：50MHz*/
	GPIO_Init(GPIOC,&GPIO_InitStruct);/*根据设定的参数初始化GPIOC*/

	GPIO_ResetBits(GPIOC,GPIO_Pin_9);/*PC9输出低电平*/

}

/*
==================================================
    继电器驱动源文件 (relay.c)

    功能：
    实现继电器的GPIO初始化配置。
    使用PC0引脚控制继电器模块。

    硬件连接：
    - RELAY : PC0 (推挽输出，高电平吸合)

    说明：
    继电器为高电平吸合(导通)，初始化时输出低电平使其断开。
==================================================
*/
#include "relay.h"



/*
    函数名：relay_init
    功能：初始化继电器对应的GPIO引脚
    参数：无
    返回值：无

    说明：
    配置PC0为推挽输出模式，速度50MHz。
    初始化后输出低电平，使继电器处于断开状态。
    后续通过RELAY宏(=1吸合, =0断开)控制继电器。
*/
void relay_init(){
	GPIO_InitTypeDef GPIO_InitStruct;/*通过结构体类型定义结构体变量*/

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);/*使能GPIOA的端口时钟*/

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;/*PIN引脚:PC0*/
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;/*工作模式：推挽输出*/
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;/*工作速度：50MHz*/
	GPIO_Init(GPIOC,&GPIO_InitStruct);/*根据设定的参数初始化GPIOC*/

	GPIO_ResetBits(GPIOC,GPIO_Pin_0);/*PC0输出低电平*/

}





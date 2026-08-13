/*
==================================================
    LED驱动源文件 (led.c)

    功能：
    实现LED的GPIO初始化配置。
    使用PA6和PA7两个引脚控制两颗LED。

    硬件连接：
    - LED1 : PA6 (推挽输出，低电平点亮)
    - LED2 : PA7 (推挽输出，低电平点亮)

    说明：
    下方注释掉的代码为寄存器直接操作方式的参考实现，
    保留供学习参考，实际使用标准库函数方式。
==================================================
*/
#include "led.h"



/*
    函数名：led_init
    功能：初始化LED对应的GPIO引脚
    参数：无
    返回值：无

    说明：
    配置PA6和PA7为推挽输出模式，速度50MHz。
    初始化后输出高电平，使LED处于熄灭状态（共阳接法，低电平点亮）。
*/
void led_init(){
	GPIO_InitTypeDef GPIO_InitStruct;/*通过结构体类型定义结构体变量*/

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);/*使能GPIOA的端口时钟*/

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;/*PIN引脚:PA6 PA7*/
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;/*工作模式：推挽输出*/
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;/*工作速度：50MHz*/
	GPIO_Init(GPIOA,&GPIO_InitStruct);/*根据设定的参数初始化GPIOA*/

	GPIO_SetBits(GPIOA,GPIO_Pin_6 | GPIO_Pin_7);/*PA6输出高电平*/

//		//通过PA6输出低电平
//		//1.开启GPIOA的时钟
//		//*(volatile uint32_t *)0x40021018 |= (1 << 2);//GPIOA时钟使能
//	
//		//2.配置PA6为推挽输出 50MHz
//		//*(volatile uint32_t *)0x40010800 &= ~(0xF << 24);//清除对应的位里面原本的数据
//		//*(volatile uint32_t *)0x40010800 |=  (0x3 << 24);//配置模式
//	
//		//3.输出低电平
//		//*(volatile uint32_t *)0x4001080C &= ~(1 << 6);//输出数据寄存器
//		//*(volatile uint32_t *)0x40010814 = (1 << 6);//位清除寄存器
//	
//	
//		
//		RCC->APB2ENR |= (1 << 2);
//		GPIOA->CRL &= ~(0xF << 24);
//		GPIOA->CRL &=  (0x3 << 24);
//		GPIOA->ODR &= ~(1 << 6);


}





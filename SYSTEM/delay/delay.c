#include "delay.h"

u8 fac_us = 0;//us延时倍乘数
u16 fac_ms = 0;//ms延时倍乘数

void delay_init(){
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);//选择时钟源：外部时钟HCLK/8
	fac_us = 72000000/8/1000000;
	fac_ms = 72000000/8/1000;
}


void delay_us(u32 nus){
	
	u32 temp;
	SysTick->LOAD = nus*fac_us;//设置重装载值
	SysTick->VAL = 0;//清零当前值
	SysTick->CTRL |= 0x00000001;//使能定时器
	do{
		temp = SysTick->CTRL;//读取CTRL寄存器的值
	}while((temp&1) && !(temp&(1<<16)));//判断计数是否完成
	SysTick->CTRL &= ~(0x00000001);//关闭定时器
	
	
	
}

void delay_ms(u32 nms){
	
	u32 temp;
	SysTick->LOAD = nms*fac_ms;
	SysTick->VAL = 0;
	SysTick->CTRL |= 0x00000001;
	do{
		temp = SysTick->CTRL;
	}while((temp&1)&& !(temp&(1<<16)));
	SysTick->CTRL &= ~(0x00000001);
	
}








/*
==================================================
    按键驱动源文件 (key.c)

    功能：
    实现按键GPIO初始化和按键扫描功能。
    支持4个按键：DOWN、LEFT、RIGHT、WKUP。

    硬件连接：
    - DOWN/LEFT/RIGHT：GPIOC的上拉输入引脚，按下为低电平
    - WKUP：GPIOA的下拉输入引脚，按下为高电平
==================================================
*/
#include "key.h"



/*
    函数名：key_init
    功能：初始化4个按键对应的GPIO引脚
    参数：无
    返回值：无

    说明：
    DOWN(PC4)、LEFT(PC1)、RIGHT(PC5)配置为上拉输入(IPU)，
    未按下时引脚为高电平，按下时为低电平。
    WKUP(PA0)配置为下拉输入(IPD)，
    未按下时引脚为低电平，按下时为高电平。
*/
void key_init(){
	GPIO_InitTypeDef GPIO_InitStruct;/*通过结构体类型定义结构体变量*/

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);/*使能GPIOA的端口时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);/*使能GPIOC的端口时钟*/

	/* 配置DOWN(PC4)、LEFT(PC1)、RIGHT(PC5)为上拉输入 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;  /* 上拉输入：默认高电平，按下变低 */
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStruct);

	/* 配置WKUP(PA0)为下拉输入 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;  /* 下拉输入：默认低电平，按下变高 */
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);

}

//不支持连按 mode:0
//支持连按   mode:1
/*
    函数名：Key_scan
    功能：扫描按键状态，返回按下的按键编号
    参数：mode - 0:不支持连按(每次按下只返回一次)，1:支持连按(长按时持续返回)
    返回值：0=无按键按下，1=DOWN，2=LEFT，3=RIGHT，4=WKUP

    说明：
    使用静态变量key_up记录按键是否已释放。
    当key_up=1(按键已释放)且检测到按键按下时，返回对应按键值并置key_up=0。
    当所有按键都释放时，重新置key_up=1，允许下次检测。
    mode=1时每次调用都强制key_up=1，从而实现连按功能。
*/
u8 Key_scan(u8 mode){
	static u8 key_up=1;   /* 按键释放标志：1=已释放可检测，0=按下中尚未释放 */
	if(mode)key_up=1;     /* 支持连按模式：每次调用都重置标志，允许持续触发 */
	/* 检测是否有按键按下：DOWN/LEFT/RIGHT为低电平有效，WKUP为高电平有效 */
	if(key_up && ((DOWN == 0)||(LEFT == 0)||(RIGHT == 0)||(WKUP == 1))){
		key_up = 0;       /* 标记按键已按下，防止重复触发(非连按模式) */
		if(DOWN == 0)return DOWN_PRES;    /* DOWN键按下 */
		else if(LEFT == 0)return LEFT_PRES;  /* LEFT键按下 */
		else if(RIGHT == 0)return RIGHT_PRES; /* RIGHT键按下 */
		else if(WKUP == 1)return WKUP_PRES;   /* WKUP键按下 */
	}
	/* 所有按键都释放时，恢复key_up标志，允许下次按键检测 */
	if((DOWN == 1)&&(LEFT == 1)&&(RIGHT == 1)&&(WKUP == 0)){
		key_up=1;
	}
	return 0;  /* 无按键按下 */
}


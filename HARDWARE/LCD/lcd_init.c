#include "lcd_init.h"
#include "delay.h"

/*================================================
 * LCD GPIO初始化
 *
 * 功能说明：
 *     初始化LCD驱动所需的GPIOB引脚（PB5~PB9），
 *     配置为推挽输出模式，并设置初始电平状态。
 *
 * 引脚分配：
 *     PB5 -> CS  （片选）
 *     PB6 -> DC  （数据/命令选择）
 *     PB7 -> RES （复位）
 *     PB8 -> SDA （数据线/MOSI）
 *     PB9 -> SCL （时钟线/SCLK）
 *
 * 参数说明：无
 * 返回值：  无
 *================================================*/
void LCD_GPIO_Init(void)
{
			GPIO_InitTypeDef GPIO_InitStructure;
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); /* 使能GPIOB时钟 */
			GPIO_InitStructure.GPIO_Pin =(GPIO_Pin_5 |GPIO_Pin_6 |GPIO_Pin_7 |GPIO_Pin_8 |GPIO_Pin_9); /* 5个控制引脚 */
			GPIO_InitStructure.GPIO_Mode =GPIO_Mode_Out_PP;/* 推挽输出模式 */
			GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz;/* 输出速度50MHz */
			GPIO_Init(GPIOB,&GPIO_InitStructure);          /* 初始化GPIOB */
			GPIO_SetBits(GPIOB,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_9); /* CS/DC/RES/SCL 初始拉高（空闲状态） */
			GPIO_ResetBits(GPIOB,GPIO_Pin_8); /* SDA 初始拉低 */
}


/******************************************************************************
      函数说明：LCD串行数据写入函数
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(u8 dat)
{
	u8 i;
	LCD_CS_Clr();             /* 拉低片选，开始通信 */
	for(i=0;i<8;i++)          /* 逐位发送8位数据（MSB先发） */
	{
		LCD_SCLK_Clr();       /* 时钟线拉低，准备数据 */
		if(dat&0x80)           /* 检查最高位 */
		{
		   LCD_MOSI_Set();    /* 最高位为1，数据线置高 */
		}
		else
		{
		   LCD_MOSI_Clr();    /* 最高位为0，数据线置低 */
		}
		LCD_SCLK_Set();       /* 时钟线拉高，上升沿锁存数据 */
		dat<<=1;              /* 左移一位，准备发送下一位 */
	}
  	LCD_CS_Set();	            /* 拉高片选，结束通信 */
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(u8 dat)
{
	LCD_Writ_Bus(dat);          /* 直接调用总线写入函数发送单字节 */
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(u16 dat)
{
	LCD_Writ_Bus(dat>>8);       /* 先写高字节 */
	LCD_Writ_Bus(dat);          /* 再写低字节（16位数据分两次发送） */
}


/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(u8 dat)
{
	LCD_DC_Clr();               /* DC拉低，表示接下来发送的是命令 */
	LCD_Writ_Bus(dat);
	LCD_DC_Set();               /* DC拉高，恢复为数据模式 */
}


/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2)
{
	if(USE_HORIZONTAL==0)        /* 竖屏模式0 */
	{
		LCD_WR_REG(0x2a);        /* 列地址设置命令 */
		LCD_WR_DATA(x1+2);       /* 列起始地址（ST7735S偏移量+2） */
		LCD_WR_DATA(x2+2);       /* 列结束地址 */
		LCD_WR_REG(0x2b);        /* 行地址设置命令 */
		LCD_WR_DATA(y1+1);       /* 行起始地址（ST7735S偏移量+1） */
		LCD_WR_DATA(y2+1);       /* 行结束地址 */
		LCD_WR_REG(0x2c);        /* 写GRAM命令，准备接收像素数据 */
	}
	else if(USE_HORIZONTAL==1)   /* 竖屏模式1（翻转） */
	{
		LCD_WR_REG(0x2a);        /* 列地址设置 */
		LCD_WR_DATA(x1+2);
		LCD_WR_DATA(x2+2);
		LCD_WR_REG(0x2b);        /* 行地址设置 */
		LCD_WR_DATA(y1+1);
		LCD_WR_DATA(y2+1);
		LCD_WR_REG(0x2c);        /* 写GRAM */
	}
	else if(USE_HORIZONTAL==2)   /* 横屏模式2 */
	{
		LCD_WR_REG(0x2a);        /* 列地址设置（横屏偏移量+1） */
		LCD_WR_DATA(x1+1);
		LCD_WR_DATA(x2+1);
		LCD_WR_REG(0x2b);        /* 行地址设置（横屏偏移量+2） */
		LCD_WR_DATA(y1+2);
		LCD_WR_DATA(y2+2);
		LCD_WR_REG(0x2c);        /* 写GRAM */
	}
	else                          /* 横屏模式3（翻转） */
	{
		LCD_WR_REG(0x2a);        /* 列地址设置 */
		LCD_WR_DATA(x1+1);
		LCD_WR_DATA(x2+1);
		LCD_WR_REG(0x2b);        /* 行地址设置 */
		LCD_WR_DATA(y1+2);
		LCD_WR_DATA(y2+2);
		LCD_WR_REG(0x2c);        /* 写GRAM */
	}
}

/*================================================
 * LCD初始化
 *
 * 功能说明：
 *     执行ST7735S LCD驱动芯片的完整初始化序列，
 *     包括硬件复位、退出休眠、配置帧率、电源序列、
 *     VCOM电压、显示方向、Gamma校正等。
 *
 * 参数说明：无
 * 返回值：  无
 *================================================*/
void LCD_Init(void)
{
	LCD_GPIO_Init();             /* 初始化GPIO引脚 */
	LCD_RES_Clr();               /* 硬件复位：拉低复位引脚 */
	delay_ms(100);               /* 保持复位100ms */
	LCD_RES_Set();               /* 释放复位 */
	delay_ms(100);               /* 等待复位完成 */

//	LCD_BLK_Set();              /* 打开背光（当前未使用） */
//  delay_ms(100);

	//************* Start Initial Sequence **********//
	LCD_WR_REG(0x11);           /* Sleep Out：退出休眠模式 */
	delay_ms(120);               /* 等待120ms让内部时钟稳定 */
	//------------------------------------ST7735S Frame Rate-----------------------------------------//
	LCD_WR_REG(0xB1);            /* 帧率控制：正常模式 */
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x3C);
	LCD_WR_REG(0xB2);            /* 帧率控制：空闲模式 */
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x3C);
	LCD_WR_REG(0xB3);            /* 帧率控制：部分模式/其他 */
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x3C);
	//------------------------------------End ST7735S Frame Rate---------------------------------//
	LCD_WR_REG(0xB4);            /* Dot inversion：点反转 */
	LCD_WR_DATA8(0x03);
	//------------------------------------ST7735S Power Sequence---------------------------------//
	LCD_WR_REG(0xC0);            /* 电源控制1 */
	LCD_WR_DATA8(0x28);
	LCD_WR_DATA8(0x08);
	LCD_WR_DATA8(0x04);
	LCD_WR_REG(0xC1);            /* 电源控制2 */
	LCD_WR_DATA8(0XC0);
	LCD_WR_REG(0xC2);            /* 电源控制3 */
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x00);
	LCD_WR_REG(0xC3);            /* 电源控制4 */
	LCD_WR_DATA8(0x8D);
	LCD_WR_DATA8(0x2A);
	LCD_WR_REG(0xC4);            /* 电源控制5 */
	LCD_WR_DATA8(0x8D);
	LCD_WR_DATA8(0xEE);
	//---------------------------------End ST7735S Power Sequence-------------------------------------//
	LCD_WR_REG(0xC5);            /* VCOM电压设置 */
	LCD_WR_DATA8(0x1A);
	LCD_WR_REG(0x36);            /* MX, MY, RGB mode：内存数据访问控制 */
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);  /* 竖屏模式0：正常方向 */
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0); /* 竖屏模式1：X/Y镜像 */
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70); /* 横屏模式2：X镜像+交换行列 */
	else LCD_WR_DATA8(0xA0);                   /* 横屏模式3：Y镜像+交换行列 */
	//------------------------------------ST7735S Gamma Sequence---------------------------------//
	LCD_WR_REG(0xE0);            /* Gamma正极性设置 */
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x22);
	LCD_WR_DATA8(0x07);
	LCD_WR_DATA8(0x0A);
	LCD_WR_DATA8(0x2E);
	LCD_WR_DATA8(0x30);
	LCD_WR_DATA8(0x25);
	LCD_WR_DATA8(0x2A);
	LCD_WR_DATA8(0x28);
	LCD_WR_DATA8(0x26);
	LCD_WR_DATA8(0x2E);
	LCD_WR_DATA8(0x3A);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x01);
	LCD_WR_DATA8(0x03);
	LCD_WR_DATA8(0x13);
	LCD_WR_REG(0xE1);            /* Gamma负极性设置 */
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x16);
	LCD_WR_DATA8(0x06);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x2D);
	LCD_WR_DATA8(0x26);
	LCD_WR_DATA8(0x23);
	LCD_WR_DATA8(0x27);
	LCD_WR_DATA8(0x27);
	LCD_WR_DATA8(0x25);
	LCD_WR_DATA8(0x2D);
	LCD_WR_DATA8(0x3B);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x01);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x13);
	//------------------------------------End ST7735S Gamma Sequence-----------------------------//
	LCD_WR_REG(0x3A);            /* 颜色模式设置：65k色（RGB565） */
	LCD_WR_DATA8(0x05);
	LCD_WR_REG(0x29);            /* Display ON：开启显示 */
} 








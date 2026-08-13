#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "sys.h"

/*================================================
 * 屏幕方向设置
 *
 * 0或1：竖屏显示（128×160）
 * 2或3：横屏显示（160×128）
 *================================================*/
#define USE_HORIZONTAL 1  /* 设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏 */


/*================================================
 * LCD分辨率定义
 *
 * 根据屏幕方向自动选择宽高
 *================================================*/
#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 128    /* 竖屏宽度 */
#define LCD_H 160    /* 竖屏高度 */

#else
#define LCD_W 160    /* 横屏宽度 */
#define LCD_H 128    /* 横屏高度 */
#endif



/*================================================
 * LCD端口定义
 *
 * 采用GPIO模拟SPI通信方式驱动ST7735S
 * 引脚分配：PB5(CS) PB6(DC) PB7(RES) PB8(SDA) PB9(SCL)
 *================================================*/

#define LCD_SCLK_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_9)/* SCL=SCLK（时钟线）拉低 */
#define LCD_SCLK_Set() GPIO_SetBits(GPIOB,GPIO_Pin_9)  /* SCL=SCLK（时钟线）拉高 */

#define LCD_MOSI_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_8)/* SDA=MOSI（数据线）拉低 */
#define LCD_MOSI_Set() GPIO_SetBits(GPIOB,GPIO_Pin_8)  /* SDA=MOSI（数据线）拉高 */

#define LCD_RES_Clr()  GPIO_ResetBits(GPIOB,GPIO_Pin_7)/* RES（复位）拉低 */
#define LCD_RES_Set()  GPIO_SetBits(GPIOB,GPIO_Pin_7)  /* RES（复位）拉高 */

#define LCD_DC_Clr()   GPIO_ResetBits(GPIOB,GPIO_Pin_6)/* DC（数据/命令选择）拉低，发送命令 */
#define LCD_DC_Set()   GPIO_SetBits(GPIOB,GPIO_Pin_6)  /* DC（数据/命令选择）拉高，发送数据 */

#define LCD_CS_Clr()   GPIO_ResetBits(GPIOB,GPIO_Pin_5)/* CS（片选信号线）拉低，选中LCD */
#define LCD_CS_Set()   GPIO_SetBits(GPIOB,GPIO_Pin_5)  /* CS（片选信号线）拉高，取消选中 */

//#define LCD_BLK_Clr()  GPIO_ResetBits(GPIOB,GPIO_Pin_5)/* BLK 背光控制（当前未使用） */
//#define LCD_BLK_Set()  GPIO_SetBits(GPIOB,GPIO_Pin_5)



void LCD_GPIO_Init(void);    /* 初始化GPIO */
void LCD_Writ_Bus(u8 dat);   /* 模拟SPI时序写入一个字节 */
void LCD_WR_DATA8(u8 dat);   /* 写入一个字节（8位数据） */
void LCD_WR_DATA(u16 dat);   /* 写入两个字节（16位数据） */
void LCD_WR_REG(u8 dat);     /* 写入一个指令 */
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2); /* 设置显示坐标范围 */
void LCD_Init(void);         /* LCD初始化 */
#endif





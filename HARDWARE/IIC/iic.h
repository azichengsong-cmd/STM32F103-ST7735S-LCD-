#ifndef __IIC_H
#define __IIC_H
#include "sys.h"


/*================================================
 * SDA方向控制宏
 *
 * 通过直接操作GPIOC的CRL寄存器来切换PC7(SDA)的模式。
 * IIC通信中SDA需要双向使用（发送时输出，接收时输入）。
 *
 * SDA_IN():  将PC7配置为上拉/下拉输入模式（模式0x8）
 * SDA_OUT(): 将PC7配置为通用推挽输出模式（模式0x7）
 *================================================*/

#define SDA_IN()  {GPIOC->CRL &= 0X0FFFFFFF;GPIOC->CRL |= 0X80000000;} /* PC7输入模式 */

#define SDA_OUT() {GPIOC->CRL &= 0X0FFFFFFF;GPIOC->CRL |= 0X70000000;} /* PC7输出模式 */


/*================================================
 * IIC总线引脚操作宏
 *
 * IIC_SDA: SDA数据线（PC7），用于输出数据
 * IIC_SCL: SCL时钟线（PC6），用于输出时钟
 * READ_SDA: 读取SDA线状态（PC7），用于输入数据
 *================================================*/

#define IIC_SDA PCout(7)   /* SDA数据线输出控制 */

#define IIC_SCL PCout(6)   /* SCL时钟线输出控制 */


#define READ_SDA PCin(7)   /* 读取SDA数据线输入状态 */


/*================================================
 * IIC函数声明
 *================================================*/

void IIC_init(void);       /* IIC初始化，配置GPIO引脚 */
void IIC_Start(void);      /* 产生IIC起始信号 */
void IIC_Stop(void);       /* 产生IIC停止信号 */
void IIC_Ack(void);        /* 发送ACK应答信号 */
void IIC_NAck(void);       /* 发送NACK非应答信号 */
u8 IIC_Wait_Ack(void);	    /* 等待从机应答，返回0=ACK，1=NACK */
void IIC_Send_Byte(u8 tx); /* 发送一个字节（MSB先发） */
u8 IIC_Read_Byte(u8 ack);  /* 读取一个字节，ack控制是否发送应答 */
void IIC_Scan(void);       /* 扫描IIC总线上的设备 */

#endif


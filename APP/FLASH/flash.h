#ifndef __FLASH_H
#define __FLASH_H

#include "sys.h"


/*
==================================================
                Flash 历史数据存储模块
==================================================

功能：
    存储温度、气压、海拔的累计平均值
    断电不丢失，上电自动恢复

存储策略：
    使用 STM32 内部 Flash 最后 2 页
    多槽追加写入 + 磨损均衡
    CRC32 校验防止断电损坏

STM32F103RC Flash 参数：
    总容量 256KB (0x08000000 ~ 0x0803FFFF)
    页大小   2KB
    程序占用 约 44KB
    使用最后 2 页存储数据，不与程序冲突
==================================================
*/



/*
==================================================
                Flash 存储地址
==================================================

Page 126: 0x0803F000 ~ 0x0803F7FF
Page 127: 0x0803F800 ~ 0x0803FFFF

共 4KB
==================================================
*/

#define FLASH_DATA_BASE         0x0803F000   /* Page 126 起始地址 */
#define FLASH_PAGE_SIZE         0x800        /* 每页大小 2KB */
#define FLASH_TOTAL_SIZE        (FLASH_PAGE_SIZE * 2)  /* 总容量 4KB（2页） */



/*
==================================================
                存储记录结构
==================================================

每条记录 40 字节

magic       : 魔术数，标识有效数据
valid       : 0xFFFF = 空槽(擦除后默认), 0x0001 = 有效
reserved    : 对齐填充
count       : 累计采样次数
temp_sum    : 温度累加和
pressure_sum: 气压累加和
altitude_sum: 海拔累加和
crc         : CRC32 校验值(覆盖前 36 字节)

布局：
    offset 0:  magic      (4 bytes)
    offset 4:  valid       (2 bytes)
    offset 6:  reserved    (2 bytes)
    offset 8:  count       (4 bytes)
    offset 12: temp_sum    (8 bytes)
    offset 20: pressure_sum(8 bytes)
    offset 28: altitude_sum(8 bytes)
    offset 36: crc         (4 bytes)
    total: 40 bytes
==================================================
*/

#define FLASH_MAGIC             0xCAFEBABE   /* 魔术数，标识有效存储记录 */
#define FLASH_RECORD_SIZE       40           /* 每条记录的大小（字节） */
#define FLASH_SLOT_NUM          (FLASH_TOTAL_SIZE / FLASH_RECORD_SIZE)  /* 总存储槽数 */

#define FLASH_SLOT_EMPTY        0xFFFF   /* 空槽标志（擦除后 Flash 默认值） */
#define FLASH_SLOT_VALID        0x0001   /* 有效槽标志 */



typedef struct
{
    uint32_t magic;           /* 魔术数，标识有效数据 */
    uint16_t valid;           /* 有效标志：0xFFFF=空槽, 0x0001=有效 */
    uint16_t reserved;        /* 对齐填充，未使用 */
    uint32_t count;           /* 累计采样次数 */
    double   temp_sum;        /* 温度累加和 */
    double   pressure_sum;    /* 气压累加和 */
    double   altitude_sum;    /* 海拔累加和 */
    uint32_t crc;             /* CRC32 校验值（覆盖前 36 字节） */

} FlashRecord_t;



/*
==================================================
                RAM 运行时数据
==================================================

flash_data 在 RAM 中维护当前累计状态

每次 Flash_Save() 会：
    1. 将新数据累加到 flash_data
    2. 将 flash_data 写入 Flash 下一个空槽

上电时 Flash_Init() 从 Flash 恢复 flash_data
==================================================
*/

typedef struct
{
    uint32_t count;           /* 累计采样次数 */
    double   temp_sum;        /* 温度累加和 */
    double   pressure_sum;    /* 气压累加和 */
    double   altitude_sum;    /* 海拔累加和 */
    uint8_t  initialized;     /* 初始化标志：1=已从 Flash 恢复 */

} FlashData_t;



extern FlashData_t flash_data;



/*
==================================================
                API 函数
==================================================
*/



/*
    Flash 初始化

    上电时调用
    扫描所有存储槽，找到最后一条有效记录
    恢复到 flash_data
*/
void Flash_Init(void);



/*
    保存一次数据

    将传入的温度/气压/海拔累加到 flash_data
    然后写入 Flash 下一个空槽

    如果所有槽已满，先擦除两页再从槽 0 开始写

    参数：
        temp      当前温度
        pressure  当前气压
        altitude  当前海拔
*/
void Flash_Save(double temp, double pressure, double altitude);



/*
    擦除所有历史数据

    擦除两页 Flash
    清零 flash_data
*/
void Flash_EraseAll(void);



/*
    获取历史平均温度

    返回 temp_sum / count
    count 为 0 时返回 0
*/
double Flash_GetTempAvg(void);



/*
    获取历史平均气压

    返回 pressure_sum / count
    count 为 0 时返回 0
*/
double Flash_GetPressureAvg(void);



/*
    获取历史平均海拔

    返回 altitude_sum / count
    count 为 0 时返回 0
*/
double Flash_GetAltitudeAvg(void);



/*
    获取累计采样次数
*/
uint32_t Flash_GetCount(void);



#endif

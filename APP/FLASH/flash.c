/*
==================================================
                Flash 历史数据存储模块实现
==================================================
功能：
    使用 STM32 内部 Flash 最后 2 页存储环境数据
    支持多槽追加写入 + 磨损均衡
    CRC32 校验防止断电导致数据损坏
    上电自动恢复历史累计数据

硬件：
    STM32F103RC，Flash 总容量 256KB
    使用 Page 126-127（0x0803F000 ~ 0x0803FFFF）
==================================================
*/
#include "flash.h"
#include "stm32f10x_flash.h"
#include "usart.h"
#include <string.h>


/*
==================================================
                全局变量

    flash_data 在 RAM 中维护当前累计状态
    每次 Flash_Save() 会将新数据累加并写入 Flash
    上电时由 Flash_Init() 从 Flash 恢复
==================================================
*/

FlashData_t flash_data;


/*
==================================================
                CRC32 计算
==================================================

多项式: 0xEDB88320 (反射形式)
初始值: 0xFFFFFFFF
最终异或: 0xFFFFFFFF

用于校验存储记录的完整性
防止断电导致的数据损坏

参数：
    data : 待校验的数据指针
    len  : 数据长度（字节）

返回值：
    CRC32 校验值
==================================================
*/

static uint32_t Flash_CRC32(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFF;
    uint32_t i;
    uint8_t  j;


    for(i = 0; i < len; i++)
    {
        crc ^= data[i];


        for(j = 0; j < 8; j++)
        {
            if(crc & 0x00000001)
            {
                crc = (crc >> 1) ^ 0xEDB88320;
            }
            else
            {
                crc >>= 1;
            }
        }
    }


    return crc ^ 0xFFFFFFFF;
}


/*
==================================================
                获取槽指针
==================================================

返回第 index 个存储槽的指针

参数：
    index : 槽号（0 ~ FLASH_SLOT_NUM-1）

返回值：
    指向该槽的 FlashRecord_t 指针

index 范围: 0 ~ FLASH_SLOT_NUM-1
==================================================
*/

static FlashRecord_t *Flash_GetSlot(uint16_t index)
{
    return (FlashRecord_t *)(
        FLASH_DATA_BASE +
        index * FLASH_RECORD_SIZE
    );
}


/*
==================================================
                查找最后一条有效记录
==================================================

从槽 0 开始扫描到最后一个槽

参数：无

返回值：
    >= 0 : 最后有效记录的槽号
    -1   : 没有找到有效记录

判断条件：
    1. magic == FLASH_MAGIC
    2. valid == FLASH_SLOT_VALID
    3. CRC32 校验通过
==================================================
*/

static int16_t Flash_FindLastValid(void)
{
    int16_t last_valid = -1;
    uint16_t i;


    for(i = 0; i < FLASH_SLOT_NUM; i++)
    {
        FlashRecord_t *rec = Flash_GetSlot(i);


        /*
            检查魔术数
        */

        if(rec->magic != FLASH_MAGIC)
        {
            /*
                遇到空槽

                由于写入是顺序的
                遇到空槽说明后面都是空的
                可以提前退出
            */

            if(rec->magic == 0xFFFFFFFF)
            {
                break;
            }


            /*
            非魔术数也非空
            数据异常 跳过
            */

            continue;
        }


        /*
            检查有效标志
        */

        if(rec->valid != FLASH_SLOT_VALID)
        {
            continue;
        }


        /*
            CRC 校验

            校验前 36 字节
            (整个结构体减去 crc 字段)
        */

        {
            uint32_t crc_calc;


            crc_calc = Flash_CRC32(
                (uint8_t *)rec,
                FLASH_RECORD_SIZE - 4
            );


            if(crc_calc != rec->crc)
            {
                printf(
                    "FLASH: Slot %d CRC ERROR\r\n",
                    i
                );

                continue;
            }
        }


        /*
            有效记录
        */

        last_valid = i;
    }


    return last_valid;
}


/*
==================================================
                查找下一个空槽
==================================================

从槽 0 开始扫描

参数：无

返回值：
    >= 0 : 空槽号
    -1   : 没有空槽(需要擦除)
==================================================
*/

static int16_t Flash_FindEmptySlot(void)
{
    uint16_t i;


    for(i = 0; i < FLASH_SLOT_NUM; i++)
    {
        FlashRecord_t *rec = Flash_GetSlot(i);


        if(rec->magic == 0xFFFFFFFF)
        {
            return i;
        }
    }


    return -1;
}


/*
==================================================
                写入一条记录到指定槽
==================================================

参数：
    slot  : 槽号
    record: 要写入的记录指针

返回值：
    0 : 成功
    1 : 失败
==================================================
*/

static uint8_t Flash_WriteSlot(
    uint16_t slot,
    FlashRecord_t *record)
{
    uint32_t addr;
    uint32_t *p;
    uint8_t  i;
    FLASH_Status status;


    /*
        计算写入地址
    */

    addr = FLASH_DATA_BASE +
           slot * FLASH_RECORD_SIZE;


    /*
        将记录转为 uint32_t 数组逐字写入

        Flash 编程必须按字(32位)写入
    */

    p = (uint32_t *)record;


    FLASH_Unlock();


    for(i = 0; i < FLASH_RECORD_SIZE / 4; i++)
    {
        status = FLASH_ProgramWord(
            addr + i * 4,
            p[i]
        );


        if(status != FLASH_COMPLETE)
        {
            printf(
                "FLASH: Write error at slot %d word %d, status=%d\r\n",
                slot, i, status
            );

            FLASH_Lock();

            return 1;
        }
    }


    FLASH_Lock();


    return 0;
}


/*
==================================================
                擦除两页 Flash
==================================================

擦除 Page 126 和 Page 127

参数：无
返回值：无

注意：
    Flash 擦除时 CPU 会停顿约 20~40ms
    期间无法从 Flash 读取代码
    因此先关闭中断，擦除完成后再开启
==================================================
*/

static void Flash_ErasePages(void)
{
    FLASH_Status status;


    printf("FLASH: Erasing pages...\r\n");


    /*
        关闭中断

        防止擦除期间中断触发
        导致 CPU 从 Flash 取指时停顿
    */

    __set_PRIMASK(1);


    FLASH_Unlock();


    /*
        擦除 Page 126
    */

    status = FLASH_ErasePage(FLASH_DATA_BASE);


    if(status != FLASH_COMPLETE)
    {
        printf("FLASH: Erase page 126 error\r\n");
    }


    /*
        擦除 Page 127
    */

    status = FLASH_ErasePage(
        FLASH_DATA_BASE + FLASH_PAGE_SIZE
    );


    if(status != FLASH_COMPLETE)
    {
        printf("FLASH: Erase page 127 error\r\n");
    }


    FLASH_Lock();


    /*
        恢复中断
    */

    __set_PRIMASK(0);


    printf("FLASH: Erase done\r\n");
}


/*
==================================================
                Flash 初始化
==================================================

上电时调用

参数：无
返回值：无

流程：
    1. 清零 RAM 数据
    2. 扫描所有槽
    3. 找到最后一条有效记录
    4. 恢复到 flash_data
==================================================
*/

void Flash_Init(void)
{
    int16_t last_valid;


    /*
        清零 RAM 数据
    */

    flash_data.count        = 0;
    flash_data.temp_sum     = 0;
    flash_data.pressure_sum = 0;
    flash_data.altitude_sum = 0;
    flash_data.initialized  = 0;


    printf("\r\n");
    printf("================================\r\n");
    printf("FLASH STORAGE INIT\r\n");
    printf("Base  = 0x%08X\r\n", FLASH_DATA_BASE);
    printf("Slots = %d\r\n", FLASH_SLOT_NUM);
    printf("Record Size = %d bytes\r\n", FLASH_RECORD_SIZE);
    printf("================================\r\n");


    /*
        查找最后一条有效记录
    */

    last_valid = Flash_FindLastValid();


    if(last_valid >= 0)
    {
        /*
            找到有效记录，恢复数据
        */

        FlashRecord_t *rec = Flash_GetSlot(last_valid);


        flash_data.count        = rec->count;
        flash_data.temp_sum     = rec->temp_sum;
        flash_data.pressure_sum = rec->pressure_sum;
        flash_data.altitude_sum = rec->altitude_sum;
        flash_data.initialized  = 1;


        printf("FLASH: Load OK\r\n");
        printf("  Slot        = %d\r\n", last_valid);
        printf("  Count       = %d\r\n", flash_data.count);
        printf("  TempSum     = %.1f\r\n", flash_data.temp_sum);
        printf("  PressureSum = %.0f\r\n", flash_data.pressure_sum);
        printf("  AltitudeSum = %.1f\r\n", flash_data.altitude_sum);
        printf("  TempAvg     = %.1f C\r\n", Flash_GetTempAvg());
        printf("  PressureAvg = %.0f Pa\r\n", Flash_GetPressureAvg());
        printf("  AltitudeAvg = %.1f m\r\n", Flash_GetAltitudeAvg());
    }
    else
    {
        /*
            没有找到有效记录

            可能是首次使用
            也可能是数据损坏
        */

        printf("FLASH: No valid data, starting fresh\r\n");
    }
}


/*
==================================================
                保存一次数据
==================================================

流程：
    1. 将新数据累加到 flash_data (RAM)
    2. 构造记录
    3. 查找空槽
    4. 如果没有空槽，擦除两页，从槽 0 开始
    5. 写入记录到 Flash

参数：
    temp      当前温度
    pressure  当前气压
    altitude  当前海拔

返回值：无
==================================================
*/

void Flash_Save(
    double temp,
    double pressure,
    double altitude)
{
    FlashRecord_t record;
    int16_t slot;
    uint8_t  ret;


    /*
        1. 累加到 RAM
    */

    flash_data.count++;
    flash_data.temp_sum     += temp;
    flash_data.pressure_sum += pressure;
    flash_data.altitude_sum += altitude;


    /*
        2. 构造记录
    */

    record.magic        = FLASH_MAGIC;
    record.valid        = FLASH_SLOT_VALID;
    record.reserved     = 0;
    record.count        = flash_data.count;
    record.temp_sum     = flash_data.temp_sum;
    record.pressure_sum = flash_data.pressure_sum;
    record.altitude_sum = flash_data.altitude_sum;


    /*
        计算 CRC

        校验前 36 字节(整个结构体减去 crc 字段)
    */

    record.crc = Flash_CRC32(
        (uint8_t *)&record,
        FLASH_RECORD_SIZE - 4
    );


    /*
        3. 查找空槽
    */

    slot = Flash_FindEmptySlot();


    /*
        4. 没有空槽，擦除后从槽 0 开始
    */

    if(slot < 0)
    {
        Flash_ErasePages();

        slot = 0;
    }


    /*
        5. 写入记录
    */

    ret = Flash_WriteSlot(slot, &record);


    if(ret == 0)
    {
        printf(
            "FLASH: Save OK, slot=%d, count=%d, Tavg=%.1f, Pavg=%.0f, Aavg=%.1f\r\n",
            slot,
            flash_data.count,
            Flash_GetTempAvg(),
            Flash_GetPressureAvg(),
            Flash_GetAltitudeAvg()
        );
    }
    else
    {
        printf(
            "FLASH: Save FAIL at slot %d\r\n",
            slot
        );
    }
}


/*
==================================================
                擦除所有历史数据
==================================================

擦除两页 Flash
清零 flash_data

参数：无
返回值：无
==================================================
*/

void Flash_EraseAll(void)
{
    Flash_ErasePages();


    flash_data.count        = 0;
    flash_data.temp_sum     = 0;
    flash_data.pressure_sum = 0;
    flash_data.altitude_sum = 0;
    flash_data.initialized  = 0;


    printf("FLASH: All data erased\r\n");
}


/*
==================================================
                获取历史平均温度
==================================================

返回值：
    temp_sum / count 的平均值
    count 为 0 时返回 0（避免除零错误）
==================================================
*/
double Flash_GetTempAvg(void)
{
    /* count 为 0 时返回 0，避免除零错误 */
    if(flash_data.count == 0)
    {
        return 0;
    }

    return flash_data.temp_sum / flash_data.count;
}


/*
==================================================
                获取历史平均气压
==================================================

返回值：
    pressure_sum / count 的平均值
    count 为 0 时返回 0（避免除零错误）
==================================================
*/
double Flash_GetPressureAvg(void)
{
    /* count 为 0 时返回 0，避免除零错误 */
    if(flash_data.count == 0)
    {
        return 0;
    }

    return flash_data.pressure_sum / flash_data.count;
}


/*
==================================================
                获取历史平均海拔
==================================================

返回值：
    altitude_sum / count 的平均值
    count 为 0 时返回 0（避免除零错误）
==================================================
*/
double Flash_GetAltitudeAvg(void)
{
    /* count 为 0 时返回 0，避免除零错误 */
    if(flash_data.count == 0)
    {
        return 0;
    }

    return flash_data.altitude_sum / flash_data.count;
}


/*
==================================================
                获取累计采样次数
==================================================

返回值：
    累计采样次数（uint32_t）
==================================================
*/
uint32_t Flash_GetCount(void)
{
    return flash_data.count;
}

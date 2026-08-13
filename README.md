# STM32F103RC 环境监测网关系统

基于 STM32F103RC 微控制器的多功能环境监测网关，集成传感器采集、LCD 显示、4G 远程上传、RS485 工业通信和 Flash 历史数据存储。

## 核心功能

- **BMP180 传感器采集** — 温度 / 气压 / 海拔数据，每 500ms 更新
- **LCD 屏幕显示** — ST7735S (128×160)，主界面 / 阈值设置界面 / 历史数据界面，按键切换
- **4G + MQTT 远程上传** — Air780E 模块对接 OneNET 云平台，每 30s 上传数据
- **心知天气 API** — HTTP 协议获取外部天气信息并显示
- **RS485 + Modbus RTU** — 作为从机响应上位机查询（功能码 03）
- **Flash 历史数据存储** — 断电不丢失，支持冷启动恢复，磨损均衡
- **阈值报警** — 温度 / 气压超限 LED 指示

## 硬件平台

| 参数 | 值 |
|------|-----|
| 芯片 | STM32F103RC（高密度型） |
| Flash | 256KB |
| SRAM | 48KB |
| 系统时钟 | 72MHz |
| 外部晶振 | 12MHz |

## 系统架构

```
┌─────────────────────────────────────────────────┐
│                  应用层 (APP)                     │
│  UI │ Sensor │ Alarm │ Max │ Weather │ Modbus  │
│         │ RS485 │ Flash                         │
├─────────────────────────────────────────────────┤
│               硬件驱动层 (HARDWARE)               │
│  LCD │ BMP180 │ IIC │ KEY │ LED │ BEEP         │
│         │ RELAY │ ONENET                        │
├─────────────────────────────────────────────────┤
│               系统支持层 (SYSTEM)                 │
│        delay │ sys │ usart                      │
└─────────────────────────────────────────────────┘
```

## 引脚映射

| 外设 | 引脚 | 说明 |
|------|------|------|
| LCD (ST7735S) | PB5(CS) PB6(DC) PB7(RES) PB8(MOSI) PB9(SCLK) | 模拟 SPI, 128×160 RGB565 |
| BMP180 | PC6(SCL) PC7(SDA) | 软件 I2C, 地址 0x77 |
| 按键 | PC1(LEFT) PC4(DOWN) PC5(RIGHT) PA0(WKUP) | 上拉/下拉输入 |
| LED | PA6(LED1) PA7(LED2) | 推挽输出, 低电平亮 |
| 蜂鸣器 | PC9 | 推挽输出 |
| 继电器 | PC0 | 推挽输出 |
| RS485 | PC8(DE/RE) PC10(TX) PC11(RX) | UART4, 9600bps, MAX3485 |
| USART1 (调试) | PA9(TX) PA10(RX) | 115200bps |
| USART2 (4G) | PA2(TX) PA3(RX) | 115200bps, 连接 Air780E |

## 关键时序

系统采用 **TIM2 定时器中断 (10ms) + 标志位轮询** 的调度模型：

| 任务 | 触发周期 | 执行内容 |
|------|---------|---------|
| 传感器数据更新 | 500ms (10ms × 50) | 读 BMP180 → 更新 Modbus 寄存器 → 报警检测 → 刷新 LCD |
| MQTT 数据上传 | 30s (10ms × 3000) | 上传温度/气压/海拔到 OneNET |
| Flash 数据保存 | 30s (随 MQTT 一起) | 累加当前数据到历史平均值，写入 Flash |

## Flash 历史数据存储机制

- **存储内容**：温度 / 气压 / 海拔的累加和 + 采样次数，平均值 = sum / count
- **存储位置**：STM32 内部 Flash 最后两页（`0x0803F000`~`0x0803FFFF`，4KB）
- **磨损均衡**：102 个槽位追加写入，写满 102 次才擦除一次（寿命延长 102 倍）
- **冷启动恢复**：上电时 `Flash_Init()` 扫描槽位，找到最后一条有效记录（magic + CRC32 校验）恢复到 RAM，无数据则从零开始
- **断电安全**：写入中断电导致 CRC 校验失败时，自动跳过损坏记录

## 软件模块

| 层次 | 模块 | 功能 |
|------|------|------|
| APP | UI | 三界面设计、按键切换、天气中英文转换 |
| APP | Sensor | BMP180 数据封装（温度/气压/海拔） |
| APP | Alarm | 阈值检测与 LED 报警 |
| APP | Max | 阈值存储结构体 |
| APP | Weather | 心知天气 HTTP API + 手动 JSON 解析 |
| APP | Modbus | RTU 从机（CRC16、功能码 03、非阻塞处理） |
| APP | RS485 | UART4 硬件驱动 + MAX3485 方向控制 |
| APP | Flash | 历史数据存储（多槽追加、磨损均衡、断电恢复） |
| HARDWARE | LCD | ST7735S 底层驱动、字符/汉字/图形显示 |
| HARDWARE | BMP180 | 温度/气压/海拔采集（含校准参数补偿） |
| HARDWARE | IIC | 软件模拟 I2C（PC6/PC7 开漏） |
| HARDWARE | KEY | 4 按键扫描（支持/不支持连按） |
| HARDWARE | LED / BEEP / RELAY | GPIO 输出驱动 |
| HARDWARE | ONENET | MQTT 协议对接 OneNET 云平台 |
| SYSTEM | usart | 双串口通信 + printf 重定向 |
| SYSTEM | delay / sys | 延时函数、位带操作 |

## 中断优先级

NVIC Group 2 分组（2 位抢占 + 2 位子优先级）：

| 中断源 | 抢占优先级 | 子优先级 | 说明 |
|--------|-----------|---------|------|
| USART2 (4G) | 0 | 3 | 最高优先级，确保串口数据不丢失 |
| UART4 (RS485) | 1 | 1 | Modbus 通信数据接收 |
| TIM2 (系统节拍) | 2 | 0 | 10ms 定时，500ms/30s 计数 |
| USART1 (调试) | 3 | 3 | 最低优先级 |

## 工程结构

```
├── APP/                    # 应用层
│   ├── ALARM/              # 报警模块
│   ├── FLASH/              # Flash 历史数据存储
│   ├── MAX/                # 阈值存储
│   ├── MODBUS/             # Modbus RTU 协议
│   ├── RS485/              # RS485 驱动
│   ├── SENSOR/             # 传感器数据封装
│   ├── UI/                 # 界面设计
│   └── WEATHER/            # 天气 API
├── HARDWARE/               # 硬件驱动层
│   ├── BEEP/               # 蜂鸣器
│   ├── BMP180/             # 气压温度传感器
│   ├── IIC/                # 软件 I2C
│   ├── KEY/                # 按键
│   ├── LCD/                # LCD 显示驱动
│   ├── LED/                # LED
│   ├── ONENET/             # OneNET 云平台
│   └── RELAY/              # 继电器
├── SYSTEM/                 # 系统支持层
│   ├── delay/              # 延时
│   ├── sys/                # 位带操作
│   └── usart/              # 串口通信
├── Project/                # 工程文件
│   ├── main.c              # 主程序
│   └── Template.uvprojx    # Keil 工程
├── CORE/                   # 内核文件
├── STM32F10X_FWLib/        # 标准库
└── STM32环境监测网关_技术开发文档.docx
```

## 编译方法

1. 使用 Keil MDK-ARM (V5) 打开 `Project/Template.uvprojx`
2. 选择编译器 ARM Compiler V5
3. 编译（Rebuild）生成 `.axf` 文件
4. 使用 ST-Link 或 J-Link 下载到 STM32F103RC

> **注意**：Flash 下载设置请选择 **Erase Sectors**（不要选 Erase Full Chip），避免擦除 Flash 最后两页的历史数据存储区。

## 五日开发计划

| 日期 | 内容 | 实现模块 |
|------|------|---------|
| Day 1 | LCD 驱动与界面设计 | lcd_init.c, lcd.c, ui.c |
| Day 2 | BMP180 驱动与阈值交互 | bmp180.c, sensor.c, max.c, alarm.c |
| Day 3 | 4G 联网与远程交互 | onenet.c, weather.c, usart.c |
| Day 4 | RS485 与 Modbus RTU | rs485.c, modbus.c |
| Day 5 | 项目融合与优化 | main.c, TIM2, flash.c |

## 技术文档

详细的技术开发文档请参考 `STM32环境监测网关_技术开发文档.docx`，包含每个模块的函数说明、关键时序分析和冷启动恢复机制。

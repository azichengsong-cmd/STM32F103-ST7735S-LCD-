/*
==================================================
                用户界面（UI）实现文件
==================================================
功能：
    实现 LCD 屏幕的界面绘制和按键交互
    包括主界面、设置界面、历史界面三个页面
    以及天气文本中英文映射显示

依赖模块：
    lcd     - LCD 底层驱动
    key     - 按键扫描
    sensor  - 传感器数据
    max     - 阈值配置
    weather - 天气数据
    flash   - 历史数据存储
==================================================
*/
#include "ui.h"
#include "lcd.h"
#include "key.h"
#include "lcd_init.h"
#include "sensor.h"
#include "stdio.h"
#include "max.h"
#include "weather.h"
#include "flash.h"
#include <string.h>




/* 当前页面状态，初始为主界面 */
UI_STATE ui_state = UI_MAIN;

/* 设置页面当前选中的修改项，初始为温度阈值 */
SETTING_ITEM setting_item = SET_TEMP_MAX;


/* 是否需要刷新界面：1=需要刷新，0=不需要刷新 */
u8 ui_refresh = 1;

/* 字符串格式化缓冲区，用于 sprintf 后显示 */
char buf[20];

/**
 * 函数功能：显示一个汉字
 *
 * LCD_ShowChinese12x12()一次只能显示一个汉字
 *
 * 参数说明：
 * x,y   ：显示坐标
 * s     ：一个汉字的两个字节编码
 * fc    ：字体颜色
 * bc    ：背景颜色
 *
 * 返回值：无
 */
static void UI_ShowOneChinese(
    u16 x,
    u16 y,
    u8 *s,
    u16 fc,
    u16 bc)
{
    LCD_ShowChinese12x12(
        x,
        y,
        s,
        fc,
        bc,
        12,
        0
    );
}



/**
 * 函数功能：根据英文天气显示中文天气
 *
 * 将心知天气 API 返回的英文天气描述
 * 映射为中文汉字在 LCD 上显示
 *
 * 例如：
 *
 * Sunny           -> 晴
 * Cloudy          -> 多云
 * Overcast        -> 阴
 * Light rain      -> 小雨
 * Moderate rain   -> 中雨
 * Heavy rain      -> 大雨
 * Thunderstorm    -> 雷雨
 * Light snow      -> 小雪
 * Moderate snow   -> 中雪
 * Heavy snow      -> 大雪
 * Fog             -> 雾
 * Haze            -> 霾
 * Dust            -> 沙尘
 * Sleet           -> 冻雨
 *
 * 参数说明：
 * x    ：显示起始 X 坐标
 * y    ：显示起始 Y 坐标
 * text ：英文天气文本指针
 *
 * 返回值：无
 */
void UI_ShowWeatherText(
    u16 x,
    u16 y,
    char *text)
{
    /* 空指针保护，防止传入 NULL 导致崩溃 */
    if(text == NULL)
    {
        return;
    }


    /*
        晴
    */

    if(strcmp(text, "Sunny") == 0 ||
       strcmp(text, "Clear") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"晴",
            GREEN,
            BLACK
        );
    }


    /*
        多云
    */

    else if(strcmp(text, "Cloudy") == 0 ||
            strcmp(text, "Partly Cloudy") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"多",
            GREEN,
            BLACK
        );

        UI_ShowOneChinese(
            x + 12,
            y,
            (u8 *)"云",
            GREEN,
            BLACK
        );
    }


    /*
        阴
    */

    else if(strcmp(text, "Overcast") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"阴",
            GREEN,
            BLACK
        );
    }


    /*
        小雨
    */

    else if(strcmp(text, "Light rain") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"小",
            GREEN,
            BLACK
        );

        UI_ShowOneChinese(
            x + 12,
            y,
            (u8 *)"雨",
            GREEN,
            BLACK
        );
    }


    /*
        中雨
    */

    else if(strcmp(text, "Moderate rain") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"中",
            GREEN,
            BLACK
        );

        UI_ShowOneChinese(
            x + 12,
            y,
            (u8 *)"雨",
            GREEN,
            BLACK
        );
    }


    /*
        大雨
    */

    else if(strcmp(text, "Heavy rain") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"大",
            GREEN,
            BLACK
        );

        UI_ShowOneChinese(
            x + 12,
            y,
            (u8 *)"雨",
            GREEN,
            BLACK
        );
    }


    /*
        雷雨
    */

    else if(strcmp(text, "Thunderstorm") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"雷",
            GREEN,
            BLACK
        );

        UI_ShowOneChinese(
            x + 12,
            y,
            (u8 *)"雨",
            GREEN,
            BLACK
        );
    }


    /*
        小雪
    */

    else if(strcmp(text, "Light snow") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"小",
            GREEN,
            BLACK
        );

        UI_ShowOneChinese(
            x + 12,
            y,
            (u8 *)"雪",
            GREEN,
            BLACK
        );
    }


    /*
        中雪
    */

    else if(strcmp(text, "Moderate snow") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"中",
            GREEN,
            BLACK
        );

        UI_ShowOneChinese(
            x + 12,
            y,
            (u8 *)"雪",
            GREEN,
            BLACK
        );
    }


    /*
        大雪
    */

    else if(strcmp(text, "Heavy snow") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"大",
            GREEN,
            BLACK
        );

        UI_ShowOneChinese(
            x + 12,
            y,
            (u8 *)"雪",
            GREEN,
            BLACK
        );
    }


    /*
        雾
    */

    else if(strcmp(text, "Fog") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"雾",
            GREEN,
            BLACK
        );
    }


    /*
        霾
    */

    else if(strcmp(text, "Haze") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"霾",
            GREEN,
            BLACK
        );
    }


    /*
        沙尘
    */

    else if(strcmp(text, "Dust") == 0 ||
            strcmp(text, "Sand") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"沙",
            GREEN,
            BLACK
        );

        UI_ShowOneChinese(
            x + 12,
            y,
            (u8 *)"尘",
            GREEN,
            BLACK
        );
    }


    /*
        冻雨
    */

    else if(strcmp(text, "Sleet") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"冻",
            GREEN,
            BLACK
        );

        UI_ShowOneChinese(
            x + 12,
            y,
            (u8 *)"雨",
            GREEN,
            BLACK
        );
    }

/*
    郑州（城市名特殊处理，用黄色显示）
*/

else if(strcmp(text, "Zhengzhou") == 0)
{
    UI_ShowOneChinese(
        x,
        y,
        (u8 *)"郑",
        YELLOW,
        BLACK
    );

    UI_ShowOneChinese(
        x + 12,
        y,
        (u8 *)"州",
        YELLOW,
        BLACK
    );
}

    /*
        冰
    */

    else if(strcmp(text, "Ice") == 0)
    {
        UI_ShowOneChinese(
            x,
            y,
            (u8 *)"冰",
            GREEN,
            BLACK
        );
    }


    /*
        如果没有匹配

        无法识别的英文文本，直接原样显示
    */

    else
    {
        LCD_ShowString(
            x,
            y,
            (u8 *)text,
            GREEN,
            BLACK,
            12,
            0
        );
    }
}





/*
==================================================
    函数功能：UI 初始化
    参数说明：无
    返回值  ：无
    说明    ：设置默认界面为主界面，
              重置设置项为温度阈值，
              标记需要刷新界面
==================================================
*/
void UI_Init(void)
{

    /* 设置为主界面 */
    ui_state = UI_MAIN;

	/* 设置项重置为温度阈值 */
	setting_item = SET_TEMP_MAX;

    /* 标记需要刷新 */
    ui_refresh = 1;

}



/*
==================================================
    函数功能：UI 显示管理
    参数说明：无
    返回值  ：无
    说明    ：根据当前界面状态调用对应的绘制函数，
              仅在 ui_refresh 为 1 时执行刷新，
              刷新完成后清除标志位
==================================================
*/
void UI_Show(void)
{


    /* 没有变化，不重新画 */
    if(ui_refresh == 0)
        return;


    /* 根据当前状态选择对应界面绘制函数 */
    switch(ui_state)
    {

        case UI_MAIN:

            UI_Main();      /* 绘制主界面 */

        break;

        case UI_SETTING:

            UI_Setting();   /* 绘制设置界面 */

        break;

        case UI_HISTORY:

            UI_History();   /* 绘制历史界面 */

        break;

    }


    /* 刷新完成，清除标志位 */
    ui_refresh = 0;

}



/*
====================================================
函数功能：
    只刷新主页面的传感器数据

刷新内容：
    温度
    气压
    海拔

注意：
    不刷新天气
    不刷新标题
    不刷新其他UI

参数说明：无
返回值  ：无
====================================================
*/
void UI_ShowSensor(void)
{
    char buf[20];
	  /*
        只有主页面才刷新传感器
    */

    if(ui_state != UI_MAIN)
    {
        return;
    }

    /*
    ================================================
        温度
    ================================================
    */

    /*
        清除温度数据区域
        先填充黑色再写入新值，避免旧数据残留
    */
    LCD_Fill(
        45,
        85,
        120,
        96,
        BLACK
    );


    /* 格式化温度：保留1位小数，单位摄氏度 */
    sprintf(
        buf,
        "%.1f C",
        sensor.temp
    );


    LCD_ShowString(
        45,
        85,
        (u8 *)buf,
        GREEN,
        BLACK,
        12,
        0
    );


    /*
    ================================================
        气压
    ================================================
    */

    /*
        清除气压数据区域
        先填充黑色再写入新值，避免旧数据残留
    */
    LCD_Fill(
        45,
        105,
        120,
        116,
        BLACK
    );


    /* 格式化气压：整数显示，单位 Pa */
    sprintf(
        buf,
        "%.0fPa",
        sensor.pressure
    );


    LCD_ShowString(
        45,
        105,
        (u8 *)buf,
        GREEN,
        BLACK,
        12,
        0
    );


    /*
    ================================================
        海拔
    ================================================
    */

    /*
        清除海拔数据区域
        先填充黑色再写入新值，避免旧数据残留
    */
    LCD_Fill(
        45,
        125,
        120,
        136,
        BLACK
    );


    /* 格式化海拔：保留1位小数，单位米 */
    sprintf(
        buf,
        "%.1fm",
        sensor.altitude
    );


    LCD_ShowString(
        45,
        125,
        (u8 *)buf,
        GREEN,
        BLACK,
        12,
        0
    );
}




/*
==================================================
    函数功能：按键处理
    参数说明：无
    返回值  ：无
    说明    ：扫描按键并执行对应操作：
              WK_UP  - 循环切换页面（主界面->设置->历史->主界面）
              DOWN   - 设置界面中切换选中项（温度/气压）
              RIGHT  - 设置界面中增加当前选中项的阈值
              LEFT   - 设置界面中减少当前选中项的阈值
==================================================
*/
void UI_Key_Handler(void)
{

    u8 key;

    /* 扫描按键，参数 0 表示不支持连按 */
    key = Key_scan(0);



    switch(key)
    {


    /* WK_UP 按键：循环切换页面 */

    case WKUP_PRES:


        /* 主界面 -> 设置界面 */
        if(ui_state == UI_MAIN)
        {

            ui_state = UI_SETTING;

        }
        /* 设置界面 -> 历史界面 */
        else if(ui_state == UI_SETTING)
        {

            ui_state = UI_HISTORY;

        }
        /* 历史界面 -> 主界面 */
        else
        {

            ui_state = UI_MAIN;

        }


        /* 页面切换后需要刷新 */
        ui_refresh = 1;


    break;



    /* DOWN 按键：设置页面选择项目 */

    case DOWN_PRES:


        if(ui_state == UI_SETTING)
        {

            /* 在温度阈值和气压阈值之间切换 */
            if(setting_item == SET_TEMP_MAX)
            {

                setting_item = SET_PRESS_MAX;

            }
            else
            {

                setting_item = SET_TEMP_MAX;

            }


            ui_refresh = 1;

        }


    break;



    /* RIGHT 按键：增加阈值 */

    case RIGHT_PRES:


        if(ui_state == UI_SETTING)
        {


            /* 增加温度阈值，上限 100 度 */
            if(setting_item == SET_TEMP_MAX)
            {

                max.temp_max += 1;
							 if(max.temp_max > 100)
								 {
							  max.temp_max = 100;/* 最高温度值不超过100 */
							 }
       

            }
            /* 增加气压阈值，上限 120000 Pa */
            else
            {

                max.pressure_max += 100;
							 if(max.pressure_max > 120000)
									{
										max.pressure_max =120000;/* 最高气压值不超过120000 */
					    	}
            }


            ui_refresh = 1;

        }


    break;



    /* LEFT 按键：减少阈值 */

    case LEFT_PRES:


        if(ui_state == UI_SETTING)
        {


            /* 减少温度阈值，下限 0 度 */
            if(setting_item == SET_TEMP_MAX)
            {

                max.temp_max -= 1;
							   if(max.temp_max < 0)
									 
								 {
									  max.temp_max =0;/* 最低温度值不超过0 */

								 }
       
            }
            /* 减少气压阈值，下限 80000 Pa */
            else
            {

                max.pressure_max -= 100;
							if(max.pressure_max < 80000)
								{
							  max.pressure_max=80000;/* 最低气压值不超过80000 */
							}
      

            }


            ui_refresh = 1;

        }


    break;


    }

}

/*=======================主界面================================
==================================================
    函数功能：主界面绘制
    参数说明：无
    返回值  ：无
    说明    ：绘制主界面，包含以下内容：
              - 标题 "ENV-MON"
              - 城市名称（中文显示）
              - 天气描述（英文转中文显示）
              - 室外温度
              - 传感器数据：温度、气压、海拔
==================================================
*/

void UI_Main(void)
{

//    char buf[30];


    /* 清屏，背景黑色 */
    LCD_Fill(0,0,LCD_W,LCD_H,BLACK);



    //================标题================

    LCD_ShowString(38,8,
                   "ENV-MON",
                   WHITE,
                   BLACK,
                   12,
                   0);


    LCD_DrawLine(0,25,127,25,WHITE);
//================实时天气信息================


/*
    城市
*/
UI_ShowWeatherText(
    5,
    35,
    weather.city
);

/*
    天气

    weather.text 例如：
    Sunny
    Cloudy
    Moderate rain
    Heavy rain

    自动转换成中文：
    晴
    多云
    中雨
    大雨
*/
UI_ShowWeatherText(
    65,
    35,
    weather.text
);


/*
    室外温度（来自天气 API 的数据）
*/
LCD_ShowString(
    5,
    55,
    "OUT:",
    WHITE,
    BLACK,
    12,
    0
);


LCD_ShowString(
    40,
    55,
    (u8 *)weather.temperature,
    GREEN,
    BLACK,
    12,
    0
);


/*
    分割线
*/
LCD_DrawLine(
    0,
    75,
    127,
    75,
    WHITE
);


    //================传感器数据================
    /* 以下为本地 BMP180 传感器数据，由 Sensor_Update() 更新 */


    //温度

    LCD_ShowChinese12x12(5,85,
                   "温",
                   WHITE,
                   BLACK,
                   12,
                   0);

    LCD_ShowChinese12x12(17,85,
                   "度",
                   WHITE,
                   BLACK,
                   12,
                   0);
		LCD_ShowString(30,85,
									 ":",
									 WHITE,
									 BLACK,
									 12,
									 0);
									 



    //气压

    LCD_ShowChinese12x12(5,105,
                   "气",
                   WHITE,
                   BLACK,
                   12,
                   0);

    LCD_ShowChinese12x12(17,105,
                   "压",
                   WHITE,
                   BLACK,
                   12,
                   0);

		LCD_ShowString(30,105,
									 ":",
									 WHITE,
									 BLACK,
									 12,
									 0);




    //海拔

    LCD_ShowChinese12x12(5,125,
                   "海",
                   WHITE,
                   BLACK,
                   12,
                   0);

    LCD_ShowChinese12x12(17,125,
                   "拔",
                   WHITE,
                   BLACK,
                   12,
                   0);

		LCD_ShowString(30,125,
									 ":",
									 WHITE,
									 BLACK,
									 12,
									 0);



}




/*====================设置温度气压阈值界面===========================
==================================================
    函数功能：设置界面绘制
    参数说明：无
    返回值  ：无
    说明    ：绘制设置界面，显示温度和气压阈值，
              当前选中项前显示 ">" 箭头标识，
              阈值用红色显示以区别于标签
==================================================
*/

void UI_Setting(void)
{

    /* 清屏，背景黑色 */
    LCD_Fill( 0, 0, LCD_W, LCD_H, BLACK);

    /* 标题 */
    LCD_ShowString(40,10, "SETTING",WHITE, BLACK,12, 0);
	
	 LCD_DrawLine(0,28,127,28,WHITE);	   //分割线
	

    LCD_ShowString(10, 40,"TEMP MAX:",WHITE,BLACK,12, 0);//最高温度
	
		sprintf(buf,"%.1f C",max.temp_max);   /* 格式化温度阈值 */

		LCD_ShowString(80,40,(u8 *)buf,RED,BLACK,12,0);


	
	  LCD_ShowString(10, 70,"PRESS MAX:",WHITE,BLACK,12, 0);//最高气压

		sprintf(buf,"%.lfPa",max.pressure_max);   /* 格式化气压阈值 */

		LCD_ShowString(80,70,(u8 *)buf,RED,BLACK,12,0);
		
		
		/* 选择箭头：在当前选中项前显示 ">" */
    if(setting_item==SET_TEMP_MAX)
    {
        LCD_ShowString(0,40,">",GREEN,BLACK,12,0);

    }
    else
    {
        LCD_ShowString(0,70,">",GREEN,BLACK,12,0);

    }

}



/*====================历史数据平均值界面===========================
==================================================
    函数功能：历史数据界面绘制
    参数说明：无
    返回值  ：无
    说明    ：从 Flash 读取历史数据并显示：
              - 历史平均温度（AVG T）
              - 历史平均气压（AVG P）
              - 历史平均海拔（AVG A）
              - 累计采样次数（CNT）
              - 底部提示按键操作
==================================================
*/

void UI_History(void)
{
    uint32_t cnt;


    /* 清屏，背景黑色 */
    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);


    //================标题================

    LCD_ShowString(34, 8, "HISTORY", WHITE, BLACK, 12, 0);

    LCD_DrawLine(0, 25, 127, 25, WHITE);


    //================平均温度================

    LCD_ShowString(5, 35, "AVG T:", WHITE, BLACK, 12, 0);

    /* 从 Flash 读取历史平均温度 */
    sprintf(buf, "%.1f C", Flash_GetTempAvg());

    LCD_ShowString(50, 35, (u8 *)buf, GREEN, BLACK, 12, 0);


    //================平均气压================

    LCD_ShowString(5, 55, "AVG P:", WHITE, BLACK, 12, 0);

    /* 从 Flash 读取历史平均气压 */
    sprintf(buf, "%.0fPa", Flash_GetPressureAvg());

    LCD_ShowString(50, 55, (u8 *)buf, GREEN, BLACK, 12, 0);


    //================平均海拔================

    LCD_ShowString(5, 75, "AVG A:", WHITE, BLACK, 12, 0);

    /* 从 Flash 读取历史平均海拔 */
    sprintf(buf, "%.1fm", Flash_GetAltitudeAvg());

    LCD_ShowString(50, 75, (u8 *)buf, GREEN, BLACK, 12, 0);


    //================分割线================

    LCD_DrawLine(0, 95, 127, 95, WHITE);


    //================采样次数================

    /* 从 Flash 读取累计采样次数 */
    cnt = Flash_GetCount();

    LCD_ShowString(5, 105, "CNT:", YELLOW, BLACK, 12, 0);

    sprintf(buf, "%d", cnt);

    LCD_ShowString(40, 105, (u8 *)buf, YELLOW, BLACK, 12, 0);


    //================提示================

    LCD_ShowString(5, 125, "WKUP:SWITCH", GRAY, BLACK, 12, 0);
}


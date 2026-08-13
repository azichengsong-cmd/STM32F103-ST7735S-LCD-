/*
==================================================
    OneNET云平台驱动源文件 (onenet.c)

    功能：
    通过Air780E 4G模块，使用AT指令实现MQTT协议连接OneNET云平台。
    支持温度、气压、海拔数据的上传。

    通信链路：
    STM32 --USART2--> Air780E --4G网络--> OneNET MQTT服务器

    连接流程(Connect_init)：
    1. 关闭旧MQTT连接 (AT+MIPCLOSE)
    2. 关闭回显 (ATE0)
    3. 查询4G网络注册状态 (AT+CGREG?)
    4. 查询数据网络附着状态 (AT+CGATT?)
    5. 配置MQTT登录参数 (AT+MCONFIG)
    6. 连接MQTT服务器 (AT+MIPSTART)
    7. MQTT登录 (AT+MCONNECT)
    8. 订阅主题 (AT+MSUB)

    数据格式：OneJSON物模型属性上报
==================================================
*/
#include "onenet.h"
#include "usart.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>



/* OneNET设备ID，用于MQTT登录标识设备身份 */
char *Device_ID = "Ai_car";


/* OneNET产品ID，标识所属产品 */
char *Product_ID = "jDa1z3yUDm";


/*
    OneNET鉴权Token
    包含版本号、资源路径、过期时间、签名方法和签名值
    用于MQTT登录时的身份验证
*/
char *Token =
"version=2018-10-31&res=products%2FjDa1z3yUDm%2Fdevices%2FAi_car&et=2028715666&method=md5&sign=A60G1pX6YLZ85P7jDHYfYQ%3D%3D";





/*
    清空USART2缓存
    功能：清空Air780E串口接收缓存区，为下一次AT指令响应做准备
    参数：无
    返回值：无

    说明：
    在发送每条AT指令前调用，确保接收缓存为空，
    避免上一次指令的残留数据干扰当前指令的响应判断。
*/

static void Receive_Clear(void)
{

    USART2_Clear();

}






/*
    等待返回数据
    功能：在指定超时时间内等待USART2接收缓存中出现指定的字符串
    参数：str - 需要匹配的字符串指针(如"OK"、"CONNECT OK"等)
         timeout - 超时时间(单位：ms)
    返回值：0=成功匹配到字符串，1=超时未匹配到

    说明：
    每1ms检查一次USART2_RX_BUF中是否包含目标字符串。
    使用strstr函数进行子串匹配。
*/

static u8 Wait_Check(char *str,u16 timeout)
{

    u16 cnt=0;


    while(cnt < timeout)
    {


        /* 在USART2接收缓存中搜索目标字符串 */
        if(strstr(USART2_RX_BUF,str)!=NULL)
        {

            return 0;  /* 匹配成功 */

        }


        delay_ms(1);  /* 延时1ms后继续检查 */


        cnt++;

    }


    return 1;  /* 超时未匹配到目标字符串 */

}







/*
    OneNET MQTT连接
    功能：通过AT指令序列完成Air780E与OneNET MQTT服务器的连接
    参数：无
    返回值：0=连接成功
           1=ATE0回显关闭失败
           2=4G网络注册查询失败
           3=数据网络附着查询失败
           4=MQTT参数配置失败
           5=MQTT服务器TCP连接失败
           6=MQTT登录失败
           7=主题订阅失败

    保留原AT命令
*/

u8 Connect_init(void)
{

    char cmd[256];


/*
    清理旧MQTT状态
    先关闭可能存在的旧连接，确保干净的初始状态
*/

Receive_Clear();


USART2_SendString(
"AT+MIPCLOSE\r\n"
);


/*
    不判断返回
    成功OK
    失败+CME ERROR
    都继续
    说明：如果之前没有连接，关闭命令会返回错误，属于正常情况，忽略即可
*/

delay_ms(500);



Receive_Clear();

    /*
        ATE0 - 关闭回显
        关闭模块的AT指令回显功能，减少串口数据量，便于解析响应
    */

    Receive_Clear();


    USART2_SendString(
        "ATE0\r\n"
    );


    if(Wait_Check("OK",100))
        return 1;  /* ATE0失败，模块无响应 */





    /*
        查询4G网络注册状态
        AT+CGREG? 检查模块是否已注册到4G网络
        返回OK表示命令执行成功(不代表已注册，仅代表命令被接受)
    */

    Receive_Clear();


    USART2_SendString(
        "AT+CGREG?\r\n"
    );


    if(Wait_Check("OK",100))
        return 2;  /* 网络注册查询失败 */





    /*
        查询数据网络附着状态
        AT+CGATT? 检查模块是否已附着到数据网络(PDP上下文已激活)
        返回OK表示命令执行成功
    */

    Receive_Clear();


    USART2_SendString(
        "AT+CGATT?\r\n"
    );


    if(Wait_Check("OK",100))
        return 3;  /* 数据网络附着查询失败 */








    /*
        配置MQTT登录参数

        AT+MCONFIG="<设备ID>","<产品ID>","<鉴权Token>"
        设置MQTT客户端的ClientID、用户名和密码

        原命令保持不变
    */


    Receive_Clear();


    /* 拼接AT+MCONFIG指令，填入设备ID、产品ID和鉴权Token */
    sprintf(cmd,
    "AT+MCONFIG=\"%s\",\"%s\",\"%s\"\r\n",
    Device_ID,
    Product_ID,
    Token);



    USART2_SendString(cmd);



    if(Wait_Check("OK",300))
        return 4;  /* MQTT参数配置失败 */








    /*
        连接MQTT服务器

        AT+MIPSTART="mqtts.heclouds.com",1883
        建立TCP连接到OneNET MQTT服务器

        原命令保持不变
    */


    Receive_Clear();



/* 建立到OneNET MQTT服务器的TCP连接 */
USART2_SendString(
"AT+MIPSTART=\"mqtts.heclouds.com\",1883\r\n"
);


/* 等待连接结果：新连接成功返回"CONNECT OK"，已连接返回"ALREADY CONNECT" */
if(
    Wait_Check("CONNECT OK",500)
)
{

}
else if(
    Wait_Check("ALREADY CONNECT",500)
)
{

}
else
{
    return 5;  /* TCP连接失败 */
}



/* MQTT登录，心跳120秒 */
USART2_SendString(
"AT+MCONNECT=1,120\r\n"
);


/* 等待登录结果：新登录成功返回"CONNACK OK"，已登录返回"ALREADY CONNECT" */
if(
Wait_Check("CONNACK OK",500)
)
{

}
else if(
Wait_Check("ALREADY CONNECT",500)
)
{

}
else
{
    return 6;  /* MQTT登录失败 */
}






    /*
        订阅主题

        AT+MSUB="$sys/<产品ID>/<设备ID>/thing/#",0
        订阅OneNET物模型的所有消息主题，QoS=0
        用于接收云平台下发的属性设置等指令

        原命令保持不变
    */


    Receive_Clear();


    /* 拼接订阅主题指令 */
    sprintf(cmd,
    "AT+MSUB=\"$sys/%s/%s/thing/#\",0\r\n",
    Product_ID,
    Device_ID);



    USART2_SendString(cmd);



    if(Wait_Check("SUBACK",500))
        return 7;  /* 主题订阅失败 */



    return 0;  /* MQTT连接全部步骤成功完成 */

}









/*
    上传温度

    功能：通过MQTT发布(PUBLISH)消息将温度数据上传到OneNET云平台
    参数：temp - 温度值(摄氏度，浮点数)
    返回值：0=上传成功，1=上传失败

    说明：
    使用AT+MPUB指令向OneNET物模型属性上报主题发布消息。
    消息体为OneJSON格式，\\22为双引号的AT指令转义。
    AT格式保持你的原格式
*/


u8 Up_Temp_Data(double temp)
{

    char cmd[256];


    Receive_Clear();



    /* 拼接AT+MPUB指令，构造OneJSON格式的温度属性上报消息 */
    sprintf(cmd,
    "AT+MPUB=\"$sys/%s/%s/thing/property/post\",0,0,"
    "\"{\\22id\\22:\\22123\\22,"
    "\\22version\\22:\\221.0\\22,"
    "\\22params\\22:{"
    "\\22temp\\22:{"
    "\\22value\\22:%.2f}}}"
    "\"\r\n",
    Product_ID,
    Device_ID,
    temp);



    USART2_SendString(cmd);



    if(Wait_Check("OK",2000))
        return 1;  /* 温度上传失败 */



    return 0;  /* 温度上传成功 */

}









/*
    上传压力
    功能：通过MQTT发布(PUBLISH)消息将气压数据上传到OneNET云平台
    参数：pressure - 气压值(hPa，浮点数)
    返回值：0=上传成功，1=上传失败

    说明：
    使用AT+MPUB指令向OneNET物模型属性上报主题发布气压数据。
    消息体格式与温度上传相同，仅属性名改为pressure。
*/


u8 Up_Pressure_Data(double pressure)
{

    char cmd[256];


    Receive_Clear();



    /* 拼接AT+MPUB指令，构造OneJSON格式的气压属性上报消息 */
    sprintf(cmd,
    "AT+MPUB=\"$sys/%s/%s/thing/property/post\",0,0,"
    "\"{\\22id\\22:\\22123\\22,"
    "\\22version\\22:\\221.0\\22,"
    "\\22params\\22:{"
    "\\22pressure\\22:{"
    "\\22value\\22:%.2f}}}"
    "\"\r\n",
    Product_ID,
    Device_ID,
    pressure);



    USART2_SendString(cmd);



    if(Wait_Check("OK",2000))
        return 1;  /* 气压上传失败 */



    return 0;  /* 气压上传成功 */

}









/*
    上传海拔
    功能：通过MQTT发布(PUBLISH)消息将海拔数据上传到OneNET云平台
    参数：altitude - 海拔值(米，浮点数)
    返回值：0=上传成功，1=上传失败

    说明：
    使用AT+MPUB指令向OneNET物模型属性上报主题发布海拔数据。
    消息体格式与温度上传相同，仅属性名改为altitude。
*/


u8 Up_Altitude_Data(double altitude)
{

    char cmd[256];


    Receive_Clear();



    /* 拼接AT+MPUB指令，构造OneJSON格式的海拔属性上报消息 */
    sprintf(cmd,
    "AT+MPUB=\"$sys/%s/%s/thing/property/post\",0,0,"
    "\"{\\22id\\22:\\22123\\22,"
    "\\22version\\22:\\221.0\\22,"
    "\\22params\\22:{"
    "\\22altitude\\22:{"
    "\\22value\\22:%.2f}}}"
    "\"\r\n",
    Product_ID,
    Device_ID,
    altitude);



    USART2_SendString(cmd);



    if(Wait_Check("OK",2000))
        return 1;  /* 海拔上传失败 */



    return 0;  /* 海拔上传成功 */

}


/*
 * @Description:esp8266板级支持包 不适用FreeRTOS
 * @Autor: Pi
 * @Date: 2022-07-08 23:39:12
 * @LastEditTime: 2022-07-14 16:12:20
 */
#include "Bsp_ESP8266.h"

/*HAL库句柄*/
extern TIM_HandleTypeDef htim12;

/*超时相关结构体*/
typedef struct
{
  uint16_t Time_Out_Max; // 100ms * 30 = 3S
  uint16_t Time_Out_Count;
  uint8_t Time_Out_Flag;
} ESP8266_Time_Str;

ESP8266_Time_Str ESP8266_Time;

/*目标回复结构体*/
typedef struct
{
  uint8_t Find_Flag;
  uint8_t *Target0;
  uint8_t *Target1;
} Reply_Target_Str;

static Reply_Target_Str Reply_Target;

/*内部使用函数*/
static void Bsp_ESP8266_Config_Process(uint8_t *Data, uint16_t Len);
static uint8_t Bsp_ESP8266_Query_Loop(void);

/**
 * @brief 发送AT消息到ESP8266
 * @param {uint8_t} *Data
 * @param {uint8_t} Len
 * @return {*}
 */
void Bsp_ESP8266_TX(uint8_t *Data, uint8_t Len)
{
  if (Len > 255 || Data == NULL)
  {
    return;
  }

  Bsp_UART_Write(&huart1, Data, Len);
  Bsp_UART_Poll_DMA_TX(&huart1);
}

/**
 * @brief 判断回复消息是否正确或超时
 * @param {uint8_t} *Data 串口数据
 * @param {uint8_t} Len 数据长度
 * @return {*}
 */
static void Bsp_ESP8266_Config_Process(uint8_t *Data, uint16_t Len)
{
  /*等待到目标语句*/
  if (strstr((char *)Data, (char *)Reply_Target.Target0) || strstr((char *)Data, (char *)Reply_Target.Target1))
  {
    /*关闭计时并重置超时计数*/
    ESP8266_Time.Time_Out_Count = 0;
    ESP8266_Time.Time_Out_Flag = 0;
    HAL_TIM_Base_Stop_IT(&htim12);

    /*目标找到*/
    Reply_Target.Find_Flag = 1;

    /*释放串口*/
    User_UART_RX_Fun = NULL;

    return;
  }
}

/**
 * @brief 堵塞查询消息是否收到 或超时
 * @return {uint8_t} 0:成功   1:失败
 */
static uint8_t Bsp_ESP8266_Query_Loop(void)
{
  do
  {
    User_UART_RX_Loop();
  } while ((ESP8266_Time.Time_Out_Flag != 1) && (Reply_Target.Find_Flag != 1));

  /*超时未找到*/
  if (ESP8266_Time.Time_Out_Flag == 1)
  {
    ESP8266_Time.Time_Out_Flag = 0;
    return 1;
  }

  /*回复正确*/
  if (Reply_Target.Find_Flag == 1)
  {
    Reply_Target.Find_Flag = 0;
    return 0;
  }

  return 1;
}

/**
 * @brief 发送指令配置模块,阻塞版
 * @param {uint8_t} *Data       发送的数据
 * @param {uint8_t} Len         发送的数据的长度
 * @param {uint8_t} *Reply0     预期返回的数据
 * @param {uint8_t} *Reply1     预期返回的数据
 * @param {uint16_t} Time_Out   等待最大值 和定时器时基有关 当前 100ms*Time_Out = 超时时间(ms)
 * @param {uint8_t} Retry       最大重试次数
 * @return {uint8_t} 0:成功   1:失败
 */
uint8_t Bsp_ESP8266_Config(uint8_t *Data, uint8_t Len, uint8_t *Reply0, uint8_t *Reply1, uint16_t Time_Out, uint8_t Retry)
{
  /*设置等待回复目标语句*/
  Reply_Target.Target0 = Reply0;
  Reply_Target.Target1 = Reply1;

  /*设置超时时间*/
  ESP8266_Time.Time_Out_Max = Time_Out;

  /*设置超时标志*/
  ESP8266_Time.Time_Out_Flag = 0;

  for (uint8_t i = 0; i < Retry; i++)
  {
    /*设置串口功能*/
    User_UART_RX_Fun = Bsp_ESP8266_Config_Process;

    Bsp_ESP8266_TX(Data, Len);

    HAL_TIM_Base_Start_IT(&htim12);
    /*成功直接返回0*/
    if (Bsp_ESP8266_Query_Loop() == 0)
    {
      return 0;
    }
  }

  return 1;
}

/**
 * @brief 放在定时器中，提供超时计数
 * @return {*}
 */
void Bsp_ESP8266_Timer(void)
{
  ESP8266_Time.Time_Out_Count++;

  /*已超时*/
  if (ESP8266_Time.Time_Out_Count >= ESP8266_Time.Time_Out_Max)
  {
    ESP8266_Time.Time_Out_Count = 0;
    ESP8266_Time.Time_Out_Flag = 1;
    HAL_TIM_Base_Stop_IT(&htim12);

    /*目标未找到*/
    Reply_Target.Find_Flag = 0;

    /*释放串口*/
    User_UART_RX_Fun = NULL;
  }
}

/**
 * @brief esp8266电源控制
 * @param {uint8_t} Enabel  1:上电    0:下电
 * @return {uint8_t} 0:成功   1:失败
 */
uint8_t Bsp_ESP8266_Power(uint8_t Enabel)
{
  if (Enabel == 0)
  {
    /*失能ESP8266 芯片*/
    HAL_GPIO_WritePin(ESP_POW_GPIO_Port, ESP_POW_Pin, GPIO_PIN_RESET);
    return 0;
  }

  uint8_t Ret = 1;

  /*设置波特兰74880*/
  Bsp_UART_Set_BRR(&huart1, 1);

  /*使能ESP8266 芯片*/
  HAL_GPIO_WritePin(ESP_POW_GPIO_Port, ESP_POW_Pin, GPIO_PIN_SET);

  /*等待收到消息*/
  Ret = Bsp_ESP8266_Config(NULL, NULL, "phy ver: 1145_0, pp ver: 10.2", NULL, 30, 1);

  /*设置波特兰115200*/
  Bsp_UART_Set_BRR(&huart1, 0);

  Ret = Bsp_ESP8266_Config(NULL, NULL, "ready", NULL, 30, 1);

  return Ret;
}

/**
 * @brief 恢复出厂设置
 * @return {*}
 */
void Bsp_ESP8266_Reset(void)
{

  /*恢复出厂设置*/
  Bsp_ESP8266_TX("AT+RESTORE\r\n", 13);

  Bsp_UART_RX_Enable(&huart1, 0);
  HAL_Delay(3000);
  Bsp_UART_RX_Enable(&huart1, 1);
}

/**
 * @brief 重启ESP8266
 * @return {*}
 */
void Bsp_ESP8266_RST(void)
{
  /*恢复出厂设置*/
  Bsp_ESP8266_TX((uint8_t *)"AT+RST\r\n", 13);

  Bsp_UART_RX_Enable(&huart1, 0);
  HAL_Delay(1000);
  Bsp_UART_RX_Enable(&huart1, 1);
}

/*
 * @Description: 重构
 * @Autor: Pi
 * @Date: 2022-07-19 21:58:01
 * @LastEditTime: 2022-07-20 01:22:34
 */
#include "Bsp_ESP8266.h"

/*目标回复结构体*/
struct
{
  uint8_t Timeout_Flag;
  uint8_t Find_Flag;
  uint8_t *Target0;
  uint8_t *Target1;
} Reply_Target;

/*ESP8266状态*/
enum 
{
  OFF,          //关机
  ON,           //开机
}ESP8266_State = OFF;

/*内部使用函数*/
static void Bsp_ESP8266_RX_Fun(uint8_t *Data, uint16_t Len);
static void Bsp_ESP8266_RX_Finished(uint8_t *Data, uint16_t Len);
static void Bsp_ESP8266_RX_None(void);

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
 * @brief 发送指令配置模块,阻塞版
 * @param {uint8_t} *Data       发送的数据
 * @param {uint8_t} Len         发送的数据的长度
 * @param {uint8_t} *Reply0     预期返回的数据
 * @param {uint8_t} *Reply1     预期返回的数据
 * @param {uint16_t} Time_Out   等待最大值 和定时器时基有关 当前 100ms*Time_Out = 超时时间(ms)
 * @param {uint8_t} Retry       最大重试次数
 * @return {uint8_t} 0:成功   1:失败
 */
uint8_t Bsp_ESP8266_Config(uint8_t *Data, uint8_t Len, uint8_t *Reply0, uint8_t *Reply1,  uint16_t Timeout , uint8_t Retry)
{
  /*设置等待目标*/
  Reply_Target.Target0 = Reply0;
  Reply_Target.Target1 = Reply1;

  /*设置超时时间*/
  User_Uart_RX_Timeout_Set(5);

  /*设置串口函数功能*/
  User_UART_RX_Fun = Bsp_ESP8266_RX_Fun;
  User_UART_RX_Finished = Bsp_ESP8266_RX_Finished;
  User_UART_RX_None = Bsp_ESP8266_RX_None;
  
  for(uint8_t i = 0 ; i < Retry ; i++)
  {
    /*发送数据*/
    Bsp_ESP8266_TX(Data , Len);
    
    /*轮询*/
    do
    {
      User_UART_RX_Loop();
    } while ((Reply_Target.Find_Flag != 1) && (Reply_Target.Timeout_Flag != 1));

     /*回复正确*/
    if(Reply_Target.Find_Flag == 1)
    {
      Reply_Target.Find_Flag = 0;
      return 0;
    }
  }

  /*设置超时时间*/
  User_Uart_RX_Timeout_Set(1);

  /*若是超时*/
  if (Reply_Target.Timeout_Flag == 1)
  {
    Reply_Target.Timeout_Flag = 0;
    return 1;
  }

  return 1;

}


/**
 * @brief 串口数据接收处理
 * @param {uint8_t} *Data
 * @param {uint8_t} Len
 * @return {*}
 */
static void Bsp_ESP8266_RX_Fun(uint8_t *Data, uint16_t Len)
{
  if(ESP8266_State == OFF)
  {
    uint8_t data[] = "ready";
    static uint8_t Target_Count = 0;

    for(uint16_t i = 0 ; i< Len ; i++)
    {
      if(Data[i] == data[Target_Count])
      {
        Target_Count++;
      }
    }

    if(Target_Count > 4)
    {
      /*目标找到*/
      Reply_Target.Find_Flag = 1;
      Target_Count = 0;
    }
  }
  /*等待到目标语句*/
  else 
  {
    if(strstr((char *)Data, (char *)Reply_Target.Target0) || strstr((char *)Data, (char *)Reply_Target.Target1))
    {
        /*目标找到*/
      Reply_Target.Find_Flag = 1;
      Reply_Target.Timeout_Flag = 0;
    }
  }

}


/**
 * @brief 串口数据接收完毕处理
 * @param {uint8_t} *Data
 * @param {uint8_t} Len
 * @return {*}
 */
static void Bsp_ESP8266_RX_Finished(uint8_t *Data, uint16_t Len)
{
  if(ESP8266_State == OFF)
  {
    uint8_t data[] = "ready";
    static uint8_t Target_Count = 0;

    for(uint16_t i = 0 ; i< Len ; i++)
    {
      if(Data[i] == data[Target_Count])
      {
        Target_Count++;
      }
    }

    if(Target_Count > 4)
    {
      /*目标找到*/
      Reply_Target.Find_Flag = 1;
      Target_Count = 0;
    }
  }
  /*等待到目标语句*/
  else 
  {
    if(strstr((char *)Data, (char *)Reply_Target.Target0) || strstr((char *)Data, (char *)Reply_Target.Target1))
    {
        /*目标找到*/
      Reply_Target.Find_Flag = 1;
      Reply_Target.Timeout_Flag = 0;
    }
    else
    {
      Reply_Target.Find_Flag = 0;
      Reply_Target.Timeout_Flag = 1;
    }
  }


}

/**
 * @brief 串口无数据
 * @return {*}
 */
static void Bsp_ESP8266_RX_None(void)
{
  Reply_Target.Timeout_Flag = 1;
}

/**
 * @brief esp8266电源控制
 * @param {uint8_t} Enabel  1:上电    0:下电
 * @return {uint8_t} 0:成功   1:失败
 */
void Bsp_ESP8266_Power(uint8_t Enabel)
{
  if (Enabel == 0)
  {
    /*失能ESP8266 芯片*/
    HAL_GPIO_WritePin(ESP_POW_GPIO_Port, ESP_POW_Pin, GPIO_PIN_RESET);

    ESP8266_State = OFF;
    return;
  }

  /*
  Bsp_UART_RX_Enable(&huart1, 0);

   
  HAL_GPIO_WritePin(ESP_POW_GPIO_Port, ESP_POW_Pin, GPIO_PIN_SET);

  ESP8266_State = ON;

  HAL_Delay(1000);

  Bsp_UART_RX_Enable(&huart1, 1);

  return 0;
  */
 // User_UART_RX_Size_Max(1);
  HAL_GPIO_WritePin(ESP_POW_GPIO_Port, ESP_POW_Pin, GPIO_PIN_SET);
  
  if(Bsp_ESP8266_Config(NULL , NULL , NULL , NULL , 50 , 1) == 0)
  {
    ESP8266_State = ON;
  }
//  User_UART_RX_Size_Max(512);
}



/**
 * @brief 恢复出厂设置
 * @return {*}
 */
void Bsp_ESP8266_Reset(void)
{
//  ESP8266_State = RST;
  uint8_t CMD[] = "AT+RESTORE\r\n";
  
  /*
  uint8_t Ret = Bsp_ESP8266_Config(CMD, sizeof(CMD), "ready", NULL, 3, 3);
  if(Ret == 0)
  {
    ESP8266_State = ON;
  }
  else
  {
    ESP8266_State = RST;
  }
  */
  Bsp_UART_RX_Enable(&huart1, 0);

  Bsp_ESP8266_TX(CMD , sizeof(CMD));

  ESP8266_State = ON;

  HAL_Delay(2000);

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

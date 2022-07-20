/*
 * @Description:esp8266板级支持包 不适用FreeRTOS
 * @Autor: Pi
 * @Date: 2022-07-08 23:39:12
 * @LastEditTime: 2022-07-19 22:14:36
 */
#include "Bsp_ESP8266.h"

/*HAL库句柄*/
extern TIM_HandleTypeDef htim12;

/*超时相关结构体*/
struct 
{
  uint16_t Max; // 100ms * 30 = 3S
  uint16_t Count;
  uint8_t  Flag;
} ESP8266_Timer;

/*目标回复结构体*/
struct
{
  uint8_t Flag;
  uint8_t *Target0;
  uint8_t *Target1;
} Reply_Target;

/*ESP8266状态*/
enum 
{
  OFF,          //关机
  ON,           //开机
  RST           //重启
}ESP8266_State = OFF;


/*内部调用函数*/
static void Bsp_ESP8266_RX_Fun(uint8_t *Data, uint16_t Len);
static void Bsp_ESP8266_RX_Finished(uint8_t *Data, uint16_t Len);
static void Bsp_ESP8266_Timer_Enable(uint8_t Enable);
static uint8_t Bsp_ESP8266_Query_Timeout(void);

/**
 * @brief ESP8266定时器服务函数
 * @return {*}
 */
void Bsp_ESP8266_Timer(void)
{
  ESP8266_Timer.Count++;

  if(ESP8266_Timer.Count > ESP8266_Timer.Max )
  {
    ESP8266_Timer.Flag = 1;
    ESP8266_Timer.Count = 0;
    HAL_TIM_Base_Stop_IT(&htim12);
    __HAL_TIM_SET_COUNTER(&htim12 , 0);
  }
}

/**
 * @brief ESP8266定时器使能
 * @param {uint8_t} Enable
 * @return {*}
 */
static void Bsp_ESP8266_Timer_Enable(uint8_t Enable)
{
  if(Enable)
  {
    __HAL_TIM_CLEAR_FLAG(&htim12 , TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(&htim12);
  }
  else
  {
    HAL_TIM_Base_Stop_IT(&htim12);
    __HAL_TIM_SET_COUNTER(&htim12 , 0);
  }
}


/**
 * @brief ESP8266定时器超时查询
 * @return {*}
 */
static uint8_t Bsp_ESP8266_Query_Timeout(void)
{
  if(ESP8266_Timer.Flag == 1)
  {
    ESP8266_Timer.Flag = 0;
    return 1;
  }
  else
  {
    return 0;
  }
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
uint8_t Bsp_ESP8266_Config(uint8_t *Data, uint8_t Len, uint8_t *Reply0, uint8_t *Reply1, uint16_t Timeout, uint8_t Retry)
{
  /*设置超时相关*/
  ESP8266_Timer.Max = Timeout;

  /*设置目标语句*/
  Reply_Target.Target0 = Reply0;
  Reply_Target.Target1 = Reply1;

  /*设置串口功能*/
  User_UART_RX_Fun = Bsp_ESP8266_RX_Fun;
  User_UART_RX_Finished = Bsp_ESP8266_RX_Finished;


  for(uint8_t i = 0 ; i < Retry ; i++)
  {
    Bsp_ESP8266_Timer_Enable(1);
    Bsp_ESP8266_TX(Data , Len);
    

    do
    {
      User_UART_RX_Loop();
    } while ((Bsp_ESP8266_Query_Timeout() != 1) && (Reply_Target.Flag != 1));

    /*超时未找到*/
    if (ESP8266_Timer.Flag == 1)
    {
      ESP8266_Timer.Flag = 0;
      return 1;
    }

    /*回复正确*/
    if (Reply_Target.Flag == 1)
    {
      Reply_Target.Flag = 0;
      return 0;
    }
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
  if(Len == 0)
  {
    return;
  }
  
  if(ESP8266_State == OFF || ESP8266_State == RST)
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
      /*关闭计时并重置超时计数*/
      ESP8266_Timer.Count = 0;
      ESP8266_Timer.Flag = 0;
      Bsp_ESP8266_Timer_Enable(0);

      /*目标找到*/
      Reply_Target.Flag = 1;

      Target_Count = 0;
    }
  }
  else if(ESP8266_State == ON)
  {
    /*等待到目标语句*/
    if (strstr((char *)Data, (char *)Reply_Target.Target0) || strstr((char *)Data, (char *)Reply_Target.Target1))
    {
      
      /*关闭计时并重置超时计数*/
      ESP8266_Timer.Count = 0;
      ESP8266_Timer.Flag = 0;
      Bsp_ESP8266_Timer_Enable(0);

      /*目标找到*/
      Reply_Target.Flag = 1;

      return;
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
  Bsp_ESP8266_RX_Fun(Data , Len);
}

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

    ESP8266_State = OFF;
    return 0;
  }

  Bsp_UART_RX_Enable(&huart1, 0);

  /*使能ESP8266 芯片*/
  HAL_GPIO_WritePin(ESP_POW_GPIO_Port, ESP_POW_Pin, GPIO_PIN_SET);

  ESP8266_State = ON;

  HAL_Delay(1000);

  Bsp_UART_RX_Enable(&huart1, 1);

  return 0;
}

/**
 * @brief 恢复出厂设置
 * @return {*}
 */
void Bsp_ESP8266_Reset(void)
{
  ESP8266_State = RST;
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



/**
 * @brief ESP8266 WIFI连接
 * @param {uint8_t} *SSID
 * @param {uint8_t} *PAW
 * @return {*}
 */
uint8_t Bsp_ESP8266_Connect_AP(uint8_t *SSID, uint8_t *PAW)
{
  /*复位模组*/
  Bsp_ESP8266_Power(0);
  HAL_Delay(300);
  Bsp_ESP8266_Power(1);

  Bsp_ESP8266_Reset();                                             //恢复出厂设置
  Bsp_ESP8266_Config("ATE1\r\n", 7, "OK", NULL, 30, 3);             //关闭回显
  Bsp_ESP8266_Config("AT+CWMODE_DEF=1\r\n", 18, "OK", NULL, 30, 3); // WIFI模式1 单station模式
  Bsp_ESP8266_Config("AT+CWAUTOCONN=1\r\n", 18, "OK", NULL, 30, 3); //自动连接路由器

  uint8_t Data[100] = {0};
  uint8_t Len = sprintf((char *)Data, "AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", SSID, PAW);
  Bsp_ESP8266_Config(Data, Len, "OK", NULL, 30, 3); //设置连接的路由器

  uint8_t Ret = Bsp_ESP8266_Config("AT+CIPSTATUS\r\n", 15, "STATUS:2", NULL, 50, 5); //等待连接成功

  return Ret;
}



/**
 * @brief 连接TCP服务器
 * @param {uint8_t} *IP
 * @param {uint8_t} Port
 * @param {uint8_t} Https_Enable
 * @return {uint8_t} 0:连接成功   1:连接失败
 */
uint8_t Bsp_ESP8266_Connect_TCP(uint8_t *IP, uint8_t Port, uint8_t Https_Enable)
{
  if (Bsp_ESP8266_Config("AT\r\n", 5, "OK", NULL, 30, 3) != 0) //测试是否正常
  {
    return 1;
  }

  if (Bsp_ESP8266_Config("AT+CIPSTATUS\r\n", 15, "STATUS:2", NULL, 50, 5) != 0) //测试是否连接上wifi
  {
    return 1;
  }

  uint8_t Data[100] = {0};
  uint8_t Len = 0;

  Bsp_ESP8266_Config("ATE0\r\n", 7, "OK", NULL, 30, 3);          //关闭回显
  Bsp_ESP8266_Config("AT+CIPMODE=0\r\n", 15, "OK", NULL, 30, 3); //非透传模式

  if (Https_Enable == 1)
  {
    Bsp_ESP8266_Config("AT+CIPSSLSIZE=4096\r\n", 21, "OK", NULL, 30, 3); //设置SSL缓存
    Len = sprintf((char *)Data, "AT+CIPSTART=\"SSL\",\"%s\",%d\r\n", IP, Port);
    Data[Len] = 0;
  }
  else
  {
    Bsp_ESP8266_Config("AT+CIPMUX=1\r\n", 14, "OK", NULL, 30, 3); //设置多连接
    Len = sprintf((char *)Data, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d\r\n", 0, IP, Port);
    Data[Len] = 0;
  }

  uint8_t flag = Bsp_ESP8266_Config(Data, Len, "CONNECT", NULL, 30, 3); //建立TCP连接

  return flag;
}



/**
 * @brief ESP8266发送GET请求      
 * @param {uint8_t} *IP
 * @param {uint8_t} *Path
 * @param {uint8_t} SSLEN
 * @return {uint8_t} 0:成功   1:失败
 */
uint8_t Bsp_ESP8266_Send_Get_Request(uint8_t *IP, uint8_t *Path, uint8_t SSLEN)
{
  uint8_t tcp_buff[100] = {0};
  uint8_t tcp_buff_Len = 0;

  uint8_t AT_Buff[200];
  uint8_t AT_Len = 0;

  uint8_t tcp_http_index = 0;

  //组合 get 指令
  tcp_buff_Len = sprintf((char *)tcp_buff, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", Path, IP);

  /*使用SSL*/
  if (SSLEN == 1)
  {
    AT_Len = sprintf((char *)AT_Buff, "AT+CIPSEND=%d\r\n", tcp_buff_Len);
  }
  else
  {
    AT_Len = sprintf((char *)AT_Buff, "AT+CIPSEND=%d,%d\r\n", tcp_http_index, tcp_buff_Len);
  }

  /*设置连接ID和数据长度*/
  if(Bsp_ESP8266_Config(AT_Buff, AT_Len, "OK", ">", 30, 3) == 0)
  {
    /*发送Get请求到服务器*/
    Bsp_ESP8266_TX(tcp_buff , tcp_buff_Len);
    return 0;
  }

  return 1;

  /*关闭连接*/
  //Bsp_ESP8266_Config("AT+CIPCLOSE=0\r\n", 16, "0,CLOSED", NULL, 30, 3); //非透传模式
}
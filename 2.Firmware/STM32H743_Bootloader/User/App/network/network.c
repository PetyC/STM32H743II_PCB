/*
 * @Description:
 * @Autor: Pi
 * @Date: 2022-07-06 21:19:14
 * @LastEditTime: 2022-07-22 00:25:57
 */
#include "network.h"

/*HAL库句柄*/
extern TIM_HandleTypeDef htim12;

/*内部使用函数*/
static void User_Networt_Timer_Enable(uint8_t Enable);

static void User_Network_Finished(uint8_t *data, uint16_t len); //串口接收数据处理完成函数指针
static void User_Network_RX_Fun(uint8_t *data, uint16_t len);   //串口接收数据处理函数指针

/*内部使用变量*/
struct
{
  uint16_t Count;
  uint16_t Max;
  uint8_t Flag;
  uint8_t Enable;
} Network_Timer = {0, 20, 0, 0};

/*公共缓存*/
#define PUBLIC_BUFF_LEN 256
uint8_t Public_Buff[PUBLIC_BUFF_LEN];
uint16_t Public_Buff_Count = 0;

uint8_t Finish_Flag = 0;

/**
 * @brief 设置默认连接路由器
 * @param {uint8_t} *SSID   连接的路由器名称
 * @param {uint8_t} *PAW    连接的路由器密码
 * @return {*}
 */
uint8_t User_Network_Connect_AP(uint8_t *SSID, uint8_t *PAW)
{
  if (Bsp_ESP8266_Connect_AP(SSID, PAW) == 0)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

/**
 * @brief 连接TCP服务器
 * @param {uint8_t} *IP
 * @param {uint8_t} Port
 * @param {uint8_t} Https_Enable
 * @return {uint8_t} 0:连接成功   1:连接失败
 */
uint8_t User_Network_Connect_Tcp(uint8_t *IP, uint8_t Port, uint8_t Https_Enable)
{
  if (Bsp_ESP8266_Connect_TCP(IP, Port, Https_Enable) == 0)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

/**
 * @brief 发送Get请求获取版本信息
 * @param {uint8_t} *IP
 * @param {uint8_t} *Bin_Path
 * @param {uint8_t} SSLEN
 * @param {uint8_t} Info
 * @return {*}
 */
uint8_t User_Network_Get_Info(uint8_t *IP, uint8_t *Info_Path, uint8_t SSLEN, Info_Str *Info)
{
  memset(Public_Buff, 0, PUBLIC_BUFF_LEN);
  Public_Buff_Count = 0;

  /*发送失败*/
  if (Bsp_ESP8266_Send_Get_Request(IP, Info_Path, SSLEN) == 1)
  {
    return 1;
  }

  User_UART_RX_Fun = User_Network_RX_Fun;
  User_UART_RX_Finished = User_Network_Finished;

  do
  {
    User_UART_RX_Loop();
  } while (Finish_Flag != 1);

  /*关闭连接*/
  Bsp_ESP8266_Connect_TCP_Close();

  *Info = User_Network_Info_Process(Public_Buff, PUBLIC_BUFF_LEN);

  return 0;
}

void User_Network_RX_Fun(uint8_t *data, uint16_t len) //串口接收数据处理函数指针
{
  memcpy(Public_Buff + Public_Buff_Count, data, len);
  Public_Buff_Count += len;
}

void User_Network_Finished(uint8_t *data, uint16_t len) //串口接收数据处理完成函数指针
{
  memcpy(Public_Buff + Public_Buff_Count, data, len);
  Public_Buff_Count += len;
  Finish_Flag = 1;
}

/**
 * @brief 下载BIN文件至MCU内置FLASH中
 * @param {uint8_t} *IP
 * @param {uint8_t} *Bin_Path
 * @param {uint8_t} SSLEN
 * @return {*}
 */
void User_Network_Down_Bin(uint8_t *IP, uint8_t *Bin_Path, uint8_t SSLEN)
{

  memset(Public_Buff, 0, PUBLIC_BUFF_LEN);
  Public_Buff_Count = 0;

  if (Bsp_ESP8266_Send_Get_Request(IP, Bin_Path, SSLEN) == 1)
  {
    return;
  }

  uint8_t Temp_Data;

  /*循环找到HTTP头部*/
  while (1)
  {
    uint8_t Data = 0;
    uint16_t Occup_Size = Bsp_UART_Get_RX_Buff_Occupy(&huart1);

    if (Occup_Size > 0)
    {
      if (Network_Timer.Enable == 1)
      {
        User_Networt_Timer_Enable(0);
      }

      Bsp_UART_Read(&huart1, &Temp_Data, 1);

      if (User_Networt_IPD_Process(Temp_Data, &Data, NULL) == 0)
      {
        if (User_Networt_HTTP_Process(Data) == 0)
        {
          break;
        }
      }
    }
    else
    {
      /*打开定时器*/
      if (Network_Timer.Enable == 0)
      {
        User_Networt_Timer_Enable(1);
      }

      /*定时器超时*/
      if (Network_Timer.Flag == 1)
      {
        Network_Timer.Flag = 0;
        Bsp_ESP8266_Connect_TCP_Close();
        return;
      }
    }
  }

  /*数据部分*/
  while (1)
  {
    uint8_t Data = 0;
    uint16_t Occup_Size = Bsp_UART_Get_RX_Buff_Occupy(&huart1);

    if(Flash_State.Error == 1)
    {
      return;
    }

    if(Flash_State.Finish == 1)
    {
      return;
    }


    if (Occup_Size > 0)
    {
      if (Network_Timer.Enable == 1)
      {
        User_Networt_Timer_Enable(0);
      }

      Bsp_UART_Read(&huart1, &Temp_Data, 1);

      if (User_Networt_IPD_Process(Temp_Data, &Data, NULL) == 0)
      {
        Public_Buff[Public_Buff_Count] = Data;
        Public_Buff_Count++;

        if (Public_Buff_Count == PUBLIC_BUFF_LEN)
        {
          Public_Buff_Count = 0;
          User_App_MCU_Flash_Updata(Public_Buff, PUBLIC_BUFF_LEN);
          memset(Public_Buff, 0, PUBLIC_BUFF_LEN);
        }
      }
    }
    else
    {
      /*打开定时器*/
      if (Network_Timer.Enable == 0)
      {
        User_Networt_Timer_Enable(1);
      }

      /*定时器超时*/
      if (Network_Timer.Flag == 1)
      {
        Network_Timer.Flag = 0;

        if (Public_Buff_Count > 0)
        {
          Public_Buff_Count = 0;
          User_App_MCU_Flash_Updata(Public_Buff, PUBLIC_BUFF_LEN);
          memset(Public_Buff, 0, PUBLIC_BUFF_LEN);
        }

        /*关闭连接*/
        Bsp_ESP8266_Connect_TCP_Close();
        return;
      }
    }
  }
}

/**
 * @brief 超时服务函数 放中断
 * @return {*}
 */
void User_Networt_Timer(void)
{
  Network_Timer.Count++;

  if (Network_Timer.Count >= Network_Timer.Max)
  {
    Network_Timer.Count = 0;
    Network_Timer.Flag = 1;
    HAL_TIM_Base_Stop_IT(&htim12);
    __HAL_TIM_SET_COUNTER(&htim12, 0);
  }
}

/**
 * @brief 定时器使能
 * @param {uint8_t} Enable
 * @return {*}
 */
static void User_Networt_Timer_Enable(uint8_t Enable)
{
  if (Enable)
  {
    Network_Timer.Count = 0;
    Network_Timer.Enable = 1;
    __HAL_TIM_CLEAR_FLAG(&htim12, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(&htim12);
  }
  else
  {
    Network_Timer.Enable = 0;
    Network_Timer.Count = 0;
    HAL_TIM_Base_Stop_IT(&htim12);
    __HAL_TIM_SET_COUNTER(&htim12, 0);
  }
}

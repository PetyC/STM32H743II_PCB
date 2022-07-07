/*
 * @Description: esp8266板级支持包 阻塞版 不适用FreeRTOS
 * @Autor: Pi
 * @Date: 2022-07-06 21:19:14
 * @LastEditTime: 2022-07-07 21:20:30
 */
#include "Bsp_Esp8266.h"

/*内部使用变量*/
static __IO uint32_t ESP8266_Tick = 0;
static __IO uint8_t ESP8266_Tick_Enable = 0;

/*内部使用函数*/
static void Bsp_ESP8266_TX(uint8_t *data, uint8_t len);
static void Bsp_ESP8266_Delay(uint32_t Delay);


/**
 * @brief ESP8266发送指令
 * @param {uint8_t} *data 数据
 * @param {uint8_t} len   长度
 * @return {*}
 */
static void Bsp_ESP8266_TX(uint8_t *data, uint8_t len)
{
  if (len > 255 || data == NULL)
  {
    return;
  }

  User_UART_Write(&huart1, data, len);
  User_UART_Poll_DMA_TX(&huart1);
}

/**
 * @brief 内部堵塞延时
 * @param {uint32_t} Delay
 * @return {*}
 */
static void Bsp_ESP8266_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}


/**
 * @brief ESP8266模块电源
 * @return {uint8_t} 0:成功   1:失败
 */
uint8_t Bsp_ESP8266_Power(uint8_t enable, uint32_t Time_Out)
{

  if (enable == 1)
  {
    uint8_t Data[255];
    uint8_t Braud_Flag = 0;

    /*修改波特率*/
    User_UART_Set_BaudRate(&huart1, 74880);

    /*使能ESP8266 芯片*/
    HAL_GPIO_WritePin(ESP_POW_GPIO_Port, ESP_POW_Pin, GPIO_PIN_SET);

    /*打开Tick使能*/
    ESP8266_Tick_Enable = 1;

    /*堵塞 */
    while (Time_Out > ESP8266_Tick)
    {
      if (User_UART_Get_RX_Buff_Occupy(&huart1) == 0)
      {
        continue;
      }

      User_UART_Read(&huart1, Data, 255);

      if (Braud_Flag == 0)
      {
        /*接收到ESP串口74880波特率数据*/
        if (strstr((char *)Data, "phy ver: 1145_0, pp ver: 10.2") != NULL)
        {
          Braud_Flag = 1;
          User_UART_Set_BaudRate(&huart1, 115200);
        }
      }
      else
      {
        /*ESP8266 上电完成*/
        if (strstr((char *)Data, "ready") != NULL)
        {
          ESP8266_Tick_Enable = 0;
          ESP8266_Tick = 0;
          return 0;
        }
      }
    }

    ESP8266_Tick_Enable = 0;
    ESP8266_Tick = 0;

    return 1;
  }
  else
  {
    HAL_GPIO_WritePin(ESP_POW_GPIO_Port, ESP_POW_Pin, GPIO_PIN_RESET);
    return 0;
  }
}



/**
 * @brief 发送指令配置模块,阻塞版
 * @param {uint8_t} *Send_Data  发送的数据
 * @param {uint8_t} Send_Len    发送的数据的长度
 * @param {uint8_t} *Returnc    预期返回的数据
 * @param {*fun}    执行一个函数
 * @param {uint8_t} Send_Max    尝试次数
 * @param {uint8_t} Send_Time   尝试间隔时间(ms)
 * @return {uint8_t}    0:成功  1:失败
 */
uint8_t Bsp_ESP8266_Config_Block(uint8_t *Send_Data, uint8_t Send_Len, uint8_t *Returnc,  uint8_t (*Fun)(uint8_t* fun_data, uint8_t fun_data_len) , uint8_t Send_Max, uint32_t Send_Time)
{
  uint8_t Data[255] = {0};
  uint8_t Data_Len = 0;
  uint8_t Send_Cnt = 0;
  uint8_t Send_Flag = 0;

  while (1)
  {
    if (Send_Flag == 0)
    {
      Bsp_ESP8266_TX(Send_Data, Send_Len);
      Send_Flag = 1;
      ESP8266_Tick_Enable = 1;
    }

    if (User_UART_Get_RX_Buff_Occupy(&huart1) > 0)
    {
      Data_Len = User_UART_Read(&huart1, Data, sizeof(Data));

      if (strstr((char *)Data, (char *)Returnc) && Returnc != NULL)
      {
        if(Fun != NULL)
        {
          Fun(Data , Data_Len);
        }

        ESP8266_Tick = 0;
        ESP8266_Tick_Enable = 0;
        return 0;
      }
    }

    if (ESP8266_Tick >= Send_Time)
    {
      ESP8266_Tick = 0;
      ESP8266_Tick_Enable = 0;
      Send_Flag = 0;
      Send_Cnt++;
    }

    if (Send_Cnt > Send_Max)
    {
      ESP8266_Tick = 0;
      ESP8266_Tick_Enable = 0;
      Send_Flag = 0;
      return 1;
    }
  }
}

/**
 * @brief 提供超时时基 需要放在1MS的中断中
 * @return {*}
 */
void Bsp_ESP8266_Tick(void)
{
  if (ESP8266_Tick_Enable != 0)
  {
    ESP8266_Tick++;
  }
}



/**
 * @brief 恢复出厂设置
 */
void Bsp_ESP8266_Recover(void)
{
  /*恢复出厂设置*/
  Bsp_ESP8266_TX("AT+RESTORE\r\n" , 13);

  Bsp_ESP8266_Delay(5000);
  /*清空FIFO*/
  while(User_UART_Get_RX_Buff_Occupy(&huart1) !=0)
  {
    uint8_t data[10];
    User_UART_Read(&huart1 , data , 10);
  }

  
}



/**
 * @brief 连接路由器 
 * @param {uint8_t} *AP_Name  连接的路由器名称
 * @param {uint8_t} *AP_PAW   连接的路由器密码
 * @return {*}
 */
void Bsp_ESP8266_Connect_Ap(uint8_t *AP_Name , uint8_t *AP_PAW)
{
  uint8_t Data[100] = {0};
  uint8_t Len = 0;

  /*复位模组*/
  Bsp_ESP8266_Power(0 , 3000);
  Bsp_ESP8266_Delay(500);
  Bsp_ESP8266_Power(1 , 3000);


  
  Bsp_ESP8266_Recover();            //恢复出厂设置
  Bsp_ESP8266_Config_Block("+++" , 3 , "+++" , NULL , 3 , 3000);    //退出透传
  Bsp_ESP8266_Config_Block("ATE0\r\n" , 6 , "OK" , NULL , 3 , 3000); //关闭回显
  Bsp_ESP8266_Config_Block("AT+CWMODE_DEF=1\r\n", 17 , "OK" , NULL , 3 , 3000); //WIFI模式1
  Bsp_ESP8266_Config_Block("AT+CWAUTOCONN=1\r\n", 17 , "OK" , NULL , 3 , 3000); //自动连接路由器

  Len = sprintf((char *)Data, "AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", AP_Name , AP_PAW);

  Bsp_ESP8266_Config_Block(Data, Len , "OK" , NULL , 3 , 3000); //设置连接的路由器

  /*复位模组*/
  Bsp_ESP8266_Power(0 , 3000);
  Bsp_ESP8266_Delay(500);
  Bsp_ESP8266_Power(1 , 3000);

}


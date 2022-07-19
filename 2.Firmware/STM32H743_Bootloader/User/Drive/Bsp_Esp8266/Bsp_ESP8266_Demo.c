/*
 * @Description: 重构
 * @Autor: Pi
 * @Date: 2022-07-19 21:58:01
 * @LastEditTime: 2022-07-19 22:15:47
 */
#include "Bsp_ESP8266_Demo.h"

/*目标回复结构体*/
struct
{
  uint8_t Timeout_Flag;
  uint8_t Find_Flag;
  uint8_t *Target0;
  uint8_t *Target1;
} Reply_Target;


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



uint8_t Bsp_ESP8266_Config(uint8_t *Data, uint8_t Len, uint8_t *Reply0, uint8_t *Reply1, uint16_t Timeout, uint8_t Retry)
{
  User_Uart_RX_Timeout_Set(50);


  Bsp_ESP8266_TX(Data , Len);
  

  do
  {
    User_UART_RX_Loop();
  } while ((Bsp_ESP8266_Query_Timeout() != 1) && (Reply_Target.Flag != 1));


  
  User_Uart_RX_Timeout_Set(1);

  return 1;

}



void Bsp_ESP8266_RX_Fun(uint8_t *Data, uint16_t Len)
{
  /*等待到目标语句*/
  if (strstr((char *)Data, (char *)Reply_Target.Target0) || strstr((char *)Data, (char *)Reply_Target.Target1))
  {
    /*目标找到*/
    Reply_Target.Find_Flag = 1;
    Reply_Target.Timeout_Flag = 0;
    return;
  }

}


void Bsp_ESP8266_RX_Finished(uint8_t *Data, uint16_t Len)
{
  /*等待到目标语句*/
  if (strstr((char *)Data, (char *)Reply_Target.Target0) || strstr((char *)Data, (char *)Reply_Target.Target1))
  {
    /*目标找到*/
    Reply_Target.Find_Flag = 1;
    Reply_Target.Timeout_Flag = 0;
    return;
  }
  else
  {
    Reply_Target.Find_Flag = 1;
    Reply_Target.Timeout_Flag = 0;
  }

}
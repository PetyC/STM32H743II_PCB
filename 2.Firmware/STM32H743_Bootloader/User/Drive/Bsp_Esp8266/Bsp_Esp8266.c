/*
 * @Description: esp8266板级支持包
 * @Autor: Pi
 * @Date: 2022-07-06 21:19:14
 * @LastEditTime: 2022-07-06 21:43:58
 */
#include "Bsp_Esp8266.h"



/**
 * @brief 给ESP8266模块上电
 * @return {*}
 */
void Bsp_ESP8266_Power_On()
{
  
  HAL_GPIO_WritePin(ESP_POW_GPIO_Port , ESP_POW_Pin , 1);


}





/*
*********************************************************************************************************
*	函 数 名: ESP8266_WaitResponse
*	功能说明: 等待ESP8266返回指定的应答字符串, 可以包含任意字符。只要接收齐全即可返回。
*	形    参: _pAckStr : 应答的字符串， 长度不得超过255
*			 _usTimeOut : 命令执行超时，0表示一直等待. >０表示超时时间，单位1ms
*	返 回 值: 1 表示成功  0 表示失败
*********************************************************************************************************



uint8_t Bsp_ESP8266_Wait_Response(char *Str, uint16_t Time_Out)
{
  uint8_t Str_Len = strlen(Str);

  if(Str_Len > 255)
  {
    return;
  }
  
}

/*
 * @Description: 
 * @Autor: Pi
 * @Date: 2022-07-08 23:39:19
 * @LastEditTime: 2022-07-12 03:13:17
 */
#ifndef BSP_ESP8266_H
#define BSP_ESP8266_H

#include "main.h"
#include "string.h"
#include "stdio.h"
#include "cString.h"
#include "Bsp_Uart.h"
#include "app_uart.h"

/*超时定时器 需要放在定时器中调用*/
void Bsp_ESP8266_Timer(void);

/*堵塞式配置ESP8266*/
uint8_t Bsp_ESP8266_Config(uint8_t *Data, uint8_t Len, uint8_t *Reply0, uint8_t *Reply1, uint16_t Time_Out , uint8_t Retry);


uint8_t Bsp_ESP8266_Power(uint8_t Enabel);

#endif

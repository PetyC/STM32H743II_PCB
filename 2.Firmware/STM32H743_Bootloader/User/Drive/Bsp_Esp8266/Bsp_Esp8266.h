/*
 * @Description: ESP8266板级支持包
 * @Autor: Pi
 * @Date: 2022-07-19 21:58:08
 * @LastEditTime: 2022-07-20 01:03:42
 */
#ifndef BSP_ESP8266_H
#define BSP_ESP8266_H

#include "main.h"
#include "string.h"
#include "stdio.h"

#include "Bsp_Uart.h"
#include "app_uart.h"


uint8_t Bsp_ESP8266_Config(uint8_t *Data, uint8_t Len, uint8_t *Reply0, uint8_t *Reply1, uint16_t Timeout , uint8_t Retry);

void Bsp_ESP8266_Power(uint8_t Enabel);
#endif

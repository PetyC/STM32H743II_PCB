/*
 * @Description: esp8266板级支持包 不适用FreeRTOS
 * @Autor: Pi
 * @Date: 2022-07-08 23:39:19
 * @LastEditTime: 2022-07-13 02:59:07
 */
#ifndef BSP_ESP8266_H
#define BSP_ESP8266_H

#include "main.h"
#include "string.h"
#include "stdio.h"

#include "Bsp_Uart.h"
#include "app_uart.h"

/*超时定时器 需要放在定时器中调用*/
void Bsp_ESP8266_Timer(void);

/*堵塞式配置ESP8266*/
uint8_t Bsp_ESP8266_Config(uint8_t *Data, uint8_t Len, uint8_t *Reply0, uint8_t *Reply1, uint16_t Time_Out , uint8_t Retry);

/*ESP8266 电源控制*/
uint8_t Bsp_ESP8266_Power(uint8_t Enabel);

/*恢复出厂设置*/
void Bsp_ESP8266_Reset(void);

/*重启ESP8266*/
void Bsp_ESP8266_RST(void);

/*设置默认连接WIFI*/
uint8_t Bsp_ESP8266_Connect_AP(uint8_t *SSID, uint8_t *PAW);

/*连接TCP服务器*/
uint8_t Bsp_ESP8266_Connect_Tcp(uint8_t *IP, uint8_t Port, uint8_t Https_Enable);

/*发送数据到ESP8266*/
void Bsp_ESP8266_TX(uint8_t *Data, uint8_t Len);
#endif

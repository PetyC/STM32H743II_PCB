/*
 * @Description: esp8266板级支持包 不适用FreeRTOS
 * @Autor: Pi
 * @Date: 2022-07-06 21:19:17
 * @LastEditTime: 2022-07-11 23:06:27
 */
#ifndef BSP_ESP8266_H
#define BSP_ESP8266_H



#include "main.h"
#include "string.h"
#include "stdio.h"

#include "Bsp_Uart.h"
#include "cString.h"


/* ESP8266上电控制*/
uint8_t Bsp_ESP8266_Power(uint8_t enable , uint32_t Time_Out);

/*发送指令配置模块,阻塞版*/
uint8_t Bsp_ESP8266_Config_Block(uint8_t *Send_Data, uint8_t Send_Len, uint8_t *Returnc,  uint8_t (*Fun)(uint8_t* fun_data, uint8_t fun_data_len) , uint8_t Send_Max, uint32_t Send_Time);

/*提供超时时基 需要放在1MS的中断中*/
void Bsp_ESP8266_Tick(void);

/*恢复出厂设置*/
void Bsp_ESP8266_Recover(void);

/*连接路由器*/
void Bsp_ESP8266_Connect_Ap(uint8_t *AP_Name , uint8_t *AP_PAW);

/*连接TCP服务器*/
uint8_t Bsp_ESP8266_Connect_Tcp(uint8_t *IP , uint8_t Port , uint8_t Https_Enable);

/*发送Get请求获取版本信息*/
uint8_t Bsp_ESP8266_Get_Info(uint8_t *IP, uint8_t *Bin_Path, uint8_t SSLEN);


uint8_t Bsp_Esp8266_Info_Handle(uint8_t *data, uint8_t len);

int Bsp_ESP8266_Resolve_Url(uint8_t *ch);
#endif

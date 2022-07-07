/*
 * @Description: esp8266板级支持包 不适用FreeRTOS
 * @Autor: Pi
 * @Date: 2022-07-06 21:19:17
 * @LastEditTime: 2022-07-07 20:44:26
 */
#ifndef BSP_ESP8266_H
#define BSP_ESP8266_H



#include "main.h"
#include "string.h"
#include "stdio.h"

#include "Bsp_Uart.h"




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







#endif

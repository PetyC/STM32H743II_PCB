/*
 * @Description: 
 * @Autor: Pi
 * @Date: 2022-07-06 21:19:17
 * @LastEditTime: 2022-07-22 00:11:59
 */
#ifndef NETWORK_H
#define NETWORK_H


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "app_uart.h"
#include "Bsp_Esp8266.h"
#include "User_config.h"
#include "Network_Analy.h"



/*设置默认连接路由器*/
uint8_t User_Network_Connect_AP(uint8_t *SSID, uint8_t *PAW);

/*连接TCP服务器*/
uint8_t User_Network_Connect_Tcp(uint8_t *IP , uint8_t Port , uint8_t Https_Enable);

/*发送Get请求获取版本信息*/
uint8_t User_Network_Get_Info(uint8_t *IP, uint8_t *Info_Path, uint8_t SSLEN, Info_Str *Info);

/*下载BIN文件至MCU内置FLASH中*/
void User_Network_Down_Bin(uint8_t *IP, uint8_t *Bin_Path, uint8_t SSLEN);

/*中断超时服务函数*/
void User_Networt_Timer(void);

#endif

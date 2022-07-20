/*
 * @Description: 
 * @Autor: Pi
 * @Date: 2022-07-06 21:19:17
 * @LastEditTime: 2022-07-20 16:49:02
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
#include "cString.h"



/*设置默认连接路由器*/
uint8_t User_Network_Connect_AP(uint8_t *SSID, uint8_t *PAW);

/*连接TCP服务器*/
uint8_t User_Network_Connect_Tcp(uint8_t *IP , uint8_t Port , uint8_t Https_Enable);

/*发送Get请求获取版本信息*/
uint8_t User_Network_Get_Info(uint8_t *IP, uint8_t *Info_Path, uint8_t SSLEN, Info_Str *Info);

/*解析Info数据*/
Info_Str User_Network_Info_Process(uint8_t *data , uint16_t len);


uint8_t User_Network_Get_Bin(uint8_t *IP, uint8_t *Bin_Path, uint8_t SSLEN);

void User_Network_Down_Flash(uint8_t *Data , uint16_t Len);




#endif

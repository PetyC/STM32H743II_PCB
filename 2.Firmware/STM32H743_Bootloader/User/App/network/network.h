/*
 * @Description: 
 * @Autor: Pi
 * @Date: 2022-07-06 21:19:17
 * @LastEditTime: 2022-07-14 03:50:12
 */
#ifndef NETWORK_H
#define NETWORK_H


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "app_uart.h"
#include "Bsp_Esp8266.h"
#include "cString.h"
#include "Bootloader.h"


typedef struct 
{
  uint8_t  Version[10];        //版本号
  uint32_t Bin_Size;           //固件大小
  uint8_t  Bin_Path[255];      //Bin文件路径        eg:ota/hardware/H7-Core/Demo.bin
  uint8_t  IP[255];            //服务器地址  eg:http://www.qiandpi.com/
  uint16_t Port;               //端口号
  uint8_t  SSLEN;              //1:ssl   0:非SSL
}Info_Str;

/*
typedef struct
{
  uint8_t Init;           //是否已初始化
  uint8_t Updata;         //是否需要升级
  Info_Str Info;          //Info数据
}System_Info_Str;
*/

/*设置默认连接路由器*/
uint8_t User_Network_Connect_AP(uint8_t *SSID, uint8_t *PAW);

/*连接TCP服务器*/
uint8_t User_Network_Connect_Tcp(uint8_t *IP , uint8_t Port , uint8_t Https_Enable);

/*发送Get请求获取版本信息*/
uint8_t User_Network_Get_Info(uint8_t *IP, uint8_t *Info_Path, uint8_t SSLEN);



Info_Str User_Network_Info_Process(uint8_t *data , uint16_t len);

void User_Network_Url_Process(uint8_t *pStr , Info_Str *Info);
#endif

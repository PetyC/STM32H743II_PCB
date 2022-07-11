/*
 * @Description: esp8266板级支持包 阻塞版 不适用FreeRTOS
 * @Autor: Pi
 * @Date: 2022-07-06 21:19:14
 * @LastEditTime: 2022-07-12 03:17:44
 */
#include "Bsp_Esp8266.h"



/**
 * @brief 恢复出厂设置
 */
void Bsp_ESP8266_Recover(void)
{
  /*恢复出厂设置*/
  Bsp_ESP8266_TX("AT+RESTORE\r\n", 13);

  Bsp_ESP8266_Delay(5000);
  /*清空FIFO*/
  while (Bsp_UART_Get_RX_Buff_Occupy(&huart1) != 0)
  {
    uint8_t data[10];
    Bsp_UART_Read(&huart1, data, 10);
  }
}

/**
 * @brief 连接路由器
 * @param {uint8_t} *AP_Name  连接的路由器名称
 * @param {uint8_t} *AP_PAW   连接的路由器密码
 * @return {*}
 */
void Bsp_ESP8266_Connect_Ap(uint8_t *AP_Name, uint8_t *AP_PAW)
{
  uint8_t Data[100] = {0};
  uint8_t Len = 0;

  /*复位模组*/
  Bsp_ESP8266_Power(0, 3000);
  Bsp_ESP8266_Delay(500);
  Bsp_ESP8266_Power(1, 3000);

  // Bsp_ESP8266_Recover();            //恢复出厂设置
  Bsp_ESP8266_Config_Block("+++", 4, "+++", NULL, 3, 3000);                 //退出透传
  Bsp_ESP8266_Config_Block("ATE0\r\n", 6, "OK", NULL, 3, 3000);             //关闭回显
  Bsp_ESP8266_Config_Block("AT+CWMODE_DEF=1\r\n", 18, "OK", NULL, 3, 3000); // WIFI模式1 单station模式
  Bsp_ESP8266_Config_Block("AT+CWAUTOCONN=1\r\n", 18, "OK", NULL, 3, 3000); //自动连接路由器

  Len = sprintf((char *)Data, "AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", AP_Name, AP_PAW);
  Bsp_ESP8266_Config_Block(Data, Len, "OK", NULL, 3, 3000); //设置连接的路由器
}

/**
 * @brief 连接TCP服务器
 * @param {uint8_t} *IP
 * @param {uint8_t} Port
 * @param {uint8_t} Https_Enable
 * @return {uint8_t} 0:连接成功   1:连接失败
 */
uint8_t Bsp_ESP8266_Connect_Tcp(uint8_t *IP, uint8_t Port, uint8_t Https_Enable)
{

  if (Bsp_ESP8266_Config_Block("AT\r\n", 5, "OK", NULL, 3, 5000) != 0) //测试是否正常
  {
    return 1;
  }

  uint8_t Data[100] = {0};
  uint8_t Len = 0;

  Bsp_ESP8266_Config_Block("ATE0\r\n", 6, "OK", NULL, 3, 3000);          //关闭回显
  Bsp_ESP8266_Config_Block("AT+CIPMODE=0\r\n", 15, "OK", NULL, 3, 3000); //非透传模式

  if (Https_Enable == 1)
  {
    Bsp_ESP8266_Config_Block("AT+CIPSSLSIZE=4096\r\n", 21, "OK", NULL, 3, 3000); //设置SSL缓存
    Len = sprintf((char *)Data, "AT+CIPSTART=\"SSL\",\"%s\",%d\r\n", IP, Port);
    Data[Len] = 0;
  }
  else
  {
    Bsp_ESP8266_Config_Block("AT+CIPMUX=1\r\n", 14, "OK", NULL, 3, 3000);             //设置多连接
    Len = sprintf((char *)Data, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d\r\n", 0, IP, Port); //建立TCP连接
    Data[Len] = 0;
  }

  uint8_t flag = Bsp_ESP8266_Config_Block(Data, Len, "CONNECT", NULL, 3, 3000);

  return flag;
}

/**
 * @brief 发送Get请求获取版本信息
 * @param {uint8_t} *IP
 * @param {uint8_t} *Bin_Path
 * @param {uint8_t} SSLEN
 * @return {*}
 */
uint8_t Bsp_ESP8266_Get_Info(uint8_t *IP, uint8_t *Bin_Path, uint8_t SSLEN)
{
  uint8_t tcp_buff[100] = {0};
  uint8_t tcp_buff_Len = 0;

  uint8_t AT_Buff[200];
  uint8_t AT_Len = 0;

  uint8_t tcp_http_index = 0;

  //组合 get 指令
  tcp_buff_Len = sprintf((char *)tcp_buff, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", Bin_Path, IP);

  /*使用SSL*/
  if (SSLEN == 1)
  {
    AT_Len = sprintf((char *)AT_Buff, "AT+CIPSEND=%d\r\n", tcp_buff_Len);
  }
  else
  {
    AT_Len = sprintf((char *)AT_Buff, "AT+CIPSEND=%d,%d\r\n", tcp_http_index, tcp_buff_Len);
  }

  /*发送数据命令*/
  Bsp_ESP8266_TX(AT_Buff, AT_Len);

  Bsp_ESP8266_Delay(20);

  /*发送Get请求到服务器*/
  Bsp_ESP8266_TX(tcp_buff, tcp_buff_Len);

  return 0;
}

/**
 * @brief 版本信息数据处理
 * @param {uint8_t} *data
 * @param {uint8_t} len
 * @return {*}
 */
uint8_t Bsp_Esp8266_Info_Handle(uint8_t *data, uint8_t len)
{
  uint8_t Version[20];
  uint8_t buff[255];
  uint8_t Temp[20];

  char *str;

  //获取版本号
  str = StrBetwString((char *)data, "\"version\":\"", "\",\"");

  if (str == NULL)
  {
    return 1;
  }

  len = strlen(str); //获取版本号长度

  /*版本号长度错误*/
  if (len > 20)
  {
    return 1;
  }

  memcpy((char *)Temp, str, len);

  //版本号不一样
  if (memcmp(str, Version, len) != 0)
  {
    cStringRestore();

    //获取下载地址 URL
    str = StrBetwString((char *)data, "url\":\"", "\",\"");

    // URL 缓存到数组
    memset(buff, 0, sizeof(buff));
    sprintf((char *)buff, "%s", str);

    if (Bsp_ESP8266_Resolve_Url(buff) == 0)
    {
      return 0;
    }
    
    //解析 URL
    /*
    if (IAPResolveUrl(buff) == 0)
    {
      //存储 url 到 flash

      iap_interface_set_update_url(IAPStructValue.buff, strlen(IAPStructValue.buff));
      cStringRestore();
      iap_interface_set_update_flage(); //设置更新标志
      iap_interface_reset_mcu();        //重启

    }
    */
  }
  else
  {
    /*版本已是最新,无需更新!*/
  }

  return 0;
}

#include "Bootloader.h"
#include <stdlib.h>


int Bsp_ESP8266_Resolve_Url(uint8_t *ch)
{

  if (ch == NULL || (strlen((char *)ch) < 5))
  {
    return -1;
  }

  uint8_t Len = 0;

  // http or https
  if (memcmp(ch, "https", 5) == 0)
  {
    System_infor.SSLEN = 1;
    Len = 5;
  }
  else if (memcmp(ch, "http", 4) == 0)
  {
    System_infor.SSLEN = 0;
    Len = 4;
  }
  else
  {
    return -2;
  }

  /*IP*/
  uint8_t *Buffer = (uint8_t *)StrBetwString((char *)ch + Len, "://", ":");

  if (Buffer != NULL)
  {
    //带端口号
    memset(System_infor.IP, 0, sizeof(System_infor.IP));
    memcpy(System_infor.IP, Buffer, strlen((char *)Buffer));

    Len = Len + 3 + strlen((char *)System_infor.IP);

    cStringRestore();

    // Port
    Buffer = (uint8_t *)StrBetwString((char *)ch + Len, ":", "/");

    if (Buffer != NULL)
    {
      if (strlen((char *)Buffer) < 6 && atoi((char *)Buffer) != 0)
      {
        System_infor.Port = atoi((char *)Buffer);
        Len = Len + 1 + strlen((char *)Buffer); // 18/19

        cStringRestore();
        // Path
        memcpy(System_infor.Bin_Path, ch + Len, sizeof(System_infor.Bin_Path) - Len);
      }
      else
      {
        return -4;
      }
    }
    else
    {
      cStringRestore();
      return -5;
    }
  }
  else
  {
    //不带端口号
    cStringRestore();
    if (System_infor.SSLEN)
    {
      System_infor.Port = 443;
    }
    else
    {
      System_infor.Port = 80;
    }

    // IP
    Buffer = (uint8_t *)StrBetwString((char *)ch + Len, "://", "/");
    if (Buffer != NULL)
    {
      memset(System_infor.IP, 0, sizeof(System_infor.IP));
      memcpy(System_infor.IP, Buffer, strlen((char *)Buffer));
      Len = Len + 3 + strlen((char *)System_infor.IP); // 14/15
    }
    else
    {
      return -6;
    }
    cStringRestore();
    // Path
    memcpy(System_infor.Bin_Path, ch + Len, sizeof(System_infor.Bin_Path) - Len);
  }


  return 0;
}

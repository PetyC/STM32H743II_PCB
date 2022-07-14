/*
 * @Description:
 * @Autor: Pi
 * @Date: 2022-07-06 21:19:14
 * @LastEditTime: 2022-07-14 19:54:38
 */
#include "network.h"

/*HAL库句柄*/
extern TIM_HandleTypeDef htim12;

/*目标回复结构体*/
typedef struct
{
  uint8_t Find_Flag;
  uint8_t *Target0;
  uint8_t *Target1;
  uint16_t Time_Out_Max; // 100ms * 30 = 3S
  uint16_t Time_Out_Count;
  uint8_t Time_Out_Flag;
} Reply_Target_Str;

static Reply_Target_Str Reply_Target;

/*数据缓存*/
#define NETWORK_BUFF_LEN 1024
uint8_t NetWork_Buff[NETWORK_BUFF_LEN];

/*内部使用函数*/
static void User_Network_Process(uint8_t *Data, uint16_t Len);
static uint8_t User_Network_Query_Loop(void);
static void User_Network_Url_Process(uint8_t *pStr, Info_Str *Info);

/**
 * @brief 判断回复消息是否正确或超时
 * @param {uint8_t} *Data 串口数据
 * @param {uint8_t} Len 数据长度
 * @return {*}
 */
static void User_Network_Process(uint8_t *Data, uint16_t Len)
{
  /*等待到目标语句*/
  if (strstr((char *)Data, (char *)Reply_Target.Target0) || strstr((char *)Data, (char *)Reply_Target.Target1))
  {
    /*关闭计时并重置超时计数*/
    Reply_Target.Time_Out_Count = 0;
    Reply_Target.Time_Out_Flag = 0;
    HAL_TIM_Base_Stop_IT(&htim12);

    /*目标找到*/
    Reply_Target.Find_Flag = 1;

    /*复制到公共缓存中*/
    memcpy(NetWork_Buff, Data, Len);

    /*释放串口*/
    User_UART_RX_Fun = NULL;

    return;
  }
}

/**
 * @brief 堵塞查询消息是否收到 或超时
 * @return {uint8_t} 0:成功   1:失败
 */
static uint8_t User_Network_Query_Loop(void)
{
  do
  {
    User_UART_RX_Loop();
  } while ((Reply_Target.Time_Out_Flag != 1) && (Reply_Target.Find_Flag != 1));

  /*超时未找到*/
  if (Reply_Target.Time_Out_Flag == 1)
  {
    Reply_Target.Time_Out_Flag = 0;
    return 1;
  }

  /*回复正确*/
  if (Reply_Target.Find_Flag == 1)
  {
    Reply_Target.Find_Flag = 0;
    return 0;
  }

  /*清空公共缓存区*/
  memset(NetWork_Buff, 0, NETWORK_BUFF_LEN);

  return 1;
}

/**
 * @brief 通过网络模块发送数据
 * @param {uint8_t} *Data       发送的数据
 * @param {uint8_t} Len         发送的数据的长度
 * @param {uint8_t} *Reply0     预期返回的数据
 * @param {uint8_t} *Reply1     预期返回的数据
 * @param {uint16_t} Time_Out   等待最大值 和定时器时基有关 当前 100ms*Time_Out = 超时时间(ms)
 * @param {uint8_t} Retry       最大重试次数
 * @return {uint8_t} 0:成功   1:失败
 */
uint8_t User_Network_TX(uint8_t *Data, uint8_t Len, uint8_t *Reply0, uint8_t *Reply1, uint16_t Time_Out, uint8_t Retry)
{
  /*清空公共缓存区*/
  memset(NetWork_Buff, 0, NETWORK_BUFF_LEN);

  /*设置等待回复目标语句*/
  Reply_Target.Target0 = Reply0;
  Reply_Target.Target1 = Reply1;

  /*设置超时时间*/
  Reply_Target.Time_Out_Max = Time_Out;

  /*设置超时标志*/
  Reply_Target.Time_Out_Flag = 0;

  for (uint8_t i = 0; i < Retry; i++)
  {
    /*设置串口功能*/
    User_UART_RX_Fun = User_Network_Process;

    Bsp_ESP8266_TX(Data, Len);

    HAL_TIM_Base_Start_IT(&htim12);
    /*成功直接返回0*/
    if (User_Network_Query_Loop() == 0)
    {
      return 0;
    }
  }

  return 1;
}

/**
 * @brief 设置默认连接路由器
 * @param {uint8_t} *SSID   连接的路由器名称
 * @param {uint8_t} *PAW    连接的路由器密码
 * @return {*}
 */
uint8_t User_Network_Connect_AP(uint8_t *SSID, uint8_t *PAW)
{
  /*复位模组*/
  Bsp_ESP8266_Power(0);
  HAL_Delay(300);
  Bsp_ESP8266_Power(1);

  Bsp_ESP8266_Reset();                                              //恢复出厂设置
  Bsp_ESP8266_Config("ATE0\r\n", 7, "OK", NULL, 30, 3);             //关闭回显
  Bsp_ESP8266_Config("AT+CWMODE_DEF=1\r\n", 18, "OK", NULL, 30, 3); // WIFI模式1 单station模式
  Bsp_ESP8266_Config("AT+CWAUTOCONN=1\r\n", 18, "OK", NULL, 30, 3); //自动连接路由器

  uint8_t Data[100] = {0};
  uint8_t Len = sprintf((char *)Data, "AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", SSID, PAW);
  Bsp_ESP8266_Config(Data, Len, "OK", NULL, 30, 3); //设置连接的路由器

  uint8_t Ret = Bsp_ESP8266_Config("AT+CIPSTATUS\r\n", 15, "STATUS:2", NULL, 50, 5); //等待连接成功

  return Ret;
}

/**
 * @brief 连接TCP服务器
 * @param {uint8_t} *IP
 * @param {uint8_t} Port
 * @param {uint8_t} Https_Enable
 * @return {uint8_t} 0:连接成功   1:连接失败
 */
uint8_t User_Network_Connect_Tcp(uint8_t *IP, uint8_t Port, uint8_t Https_Enable)
{
  if (Bsp_ESP8266_Config("AT\r\n", 5, "OK", NULL, 30, 3) != 0) //测试是否正常
  {
    return 1;
  }

  if (Bsp_ESP8266_Config("AT+CIPSTATUS\r\n", 15, "STATUS:2", NULL, 50, 5) != 0) //测试是否连接上wifi
  {
    return 1;
  }

  uint8_t Data[100] = {0};
  uint8_t Len = 0;

  Bsp_ESP8266_Config("ATE0\r\n", 7, "OK", NULL, 30, 3);          //关闭回显
  Bsp_ESP8266_Config("AT+CIPMODE=0\r\n", 15, "OK", NULL, 30, 3); //非透传模式

  if (Https_Enable == 1)
  {
    Bsp_ESP8266_Config("AT+CIPSSLSIZE=4096\r\n", 21, "OK", NULL, 30, 3); //设置SSL缓存
    Len = sprintf((char *)Data, "AT+CIPSTART=\"SSL\",\"%s\",%d\r\n", IP, Port);
    Data[Len] = 0;
  }
  else
  {
    Bsp_ESP8266_Config("AT+CIPMUX=1\r\n", 14, "OK", NULL, 30, 3); //设置多连接
    Len = sprintf((char *)Data, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d\r\n", 0, IP, Port);
    Data[Len] = 0;
  }

  uint8_t flag = Bsp_ESP8266_Config(Data, Len, "CONNECT", NULL, 30, 3); //建立TCP连接

  return flag;
}

/**
 * @brief 发送Get请求获取版本信息
 * @param {uint8_t} *IP
 * @param {uint8_t} *Bin_Path
 * @param {uint8_t} SSLEN
 * @return {*}
 */
uint8_t User_Network_Get_Info(uint8_t *IP, uint8_t *Info_Path, uint8_t SSLEN)
{
  uint8_t tcp_buff[100] = {0};
  uint8_t tcp_buff_Len = 0;

  uint8_t AT_Buff[200];
  uint8_t AT_Len = 0;

  uint8_t tcp_http_index = 0;

  //组合 get 指令
  tcp_buff_Len = sprintf((char *)tcp_buff, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", Info_Path, IP);

  /*使用SSL*/
  if (SSLEN == 1)
  {
    AT_Len = sprintf((char *)AT_Buff, "AT+CIPSEND=%d\r\n", tcp_buff_Len);
  }
  else
  {
    AT_Len = sprintf((char *)AT_Buff, "AT+CIPSEND=%d,%d\r\n", tcp_http_index, tcp_buff_Len);
  }

  /*设置连接ID和数据长度*/
  Bsp_ESP8266_Config(AT_Buff, AT_Len, "OK", ">", 30, 3);

  /*发送Get请求到服务器*/
  if (User_Network_TX(tcp_buff, tcp_buff_Len, "Accept-Ranges: bytes", NULL, 30, 3) == 0)
  {
    /*关闭连接*/
    Bsp_ESP8266_Config("AT+CIPCLOSE=0\r\n", 16, "0,CLOSED", NULL, 30, 3); //非透传模式
    /*解析数据*/
    User_Network_Info_Process(NetWork_Buff, NETWORK_BUFF_LEN);
  }

  return 0;
}

/**
 * @brief Inof文本数据解析
 * @param {uint8_t} *data   Info数据
 * @param {uint16_t} len    数据长度
 * @return {*}
 */
Info_Str User_Network_Info_Process(uint8_t *data, uint16_t len)
{

  Info_Str Info = {0};
  char *pStr;

  /*获取版本号*/
  pStr = StrBetwString((char *)data, "\"Version\":\"", "\",\"");

  /*获取版本号长度*/
  uint8_t pStr_len = strlen(pStr);

  /*判断版本号是否合法*/
  if (pStr_len > 20 || pStr == NULL)
  {
    return Info;
  }

  /*缓存版本号到结构体*/
  sprintf((char *)Info.Version, "%s", pStr);

  cStringRestore();

  /*获取Bin文件大小*/
  pStr = StrBetwString((char *)data, "\"Size\":\"", "\",\"");

  /*缓存Bin文件大小到结构体*/
  Info.Bin_Size = atoi(pStr);

  cStringRestore();

  /*服务器文件存放路径 */
  pStr = StrBetwString((char *)data, "\"IP\":\"", "\",\"");

  cStringRestore();

  /*Url数据处理*/
  User_Network_Url_Process((uint8_t *)pStr, &Info);

  /*Info文件存放路径 */
  pStr = StrBetwString((char *)data, "\"Info_Path\":\"", ".txt\"");

  /*存入结构体*/
  sprintf((char *)Info.Info_Path, "%s.txt", pStr);

  cStringRestore();

  /*Bin文件存放路径 */
  pStr = StrBetwString((char *)data, "\"Bin_Path\":\"", ".bin\"");

  /*存入结构体*/
  sprintf((char *)Info.Bin_Path, "%s.bin", pStr);

  cStringRestore();
  return Info;
}

/**
 * @brief Url数据处理
 * @param {uint8_t} *pStr   Url字符串
 * @param {Info_Str} *Info  返回数据结构体
 * @return {*}
 */
static void User_Network_Url_Process(uint8_t *pStr, Info_Str *Info)
{
  if (pStr == NULL || ((strlen((char *)pStr) < 5)))
  {
    return;
  }

  uint8_t pStr_Pos = 0;

  /*https */
  if (memcmp(pStr, "https", 5) == 0)
  {
    Info->SSLEN = 1;
    pStr_Pos = 5;
  }
  /*http*/
  else if (memcmp(pStr, "http", 4) == 0)
  {
    Info->SSLEN = 0;
    pStr_Pos = 4;
  }

  /*IP地址*/
  uint8_t *pIP;

  /*假设带端口号*/
  pIP = (uint8_t *)StrBetwString((char *)pStr + pStr_Pos, "://", ":");

  if (pIP != NULL)
  {
    /*缓存IP*/
    sprintf((char *)Info->IP, "%s", pIP);

    cStringRestore();

    /*处理位置更新*/
    pStr_Pos = pStr_Pos + 3 + strlen((char *)Info->IP) + 1;

    /*缓存端口号*/
    Info->Port = atoi((char *)pStr + pStr_Pos);
  }
  else //若为空则不带端口号
  {
    cStringRestore();

    /*缓存IP*/
    sprintf((char *)Info->IP, "%s", (char *)pStr + pStr_Pos + 3);

    /*默认端口80*/
    Info->Port = 80;
  }
}




#include "Fifo.h"
#define HTTP_BUFFER_LEN   1024
uint8_t Http_Buffer[HTTP_BUFFER_LEN];
_fifo_t Http_RX_fifo;

/* fifo上锁函数 */
static void fifo_lock(void)
{
  __disable_irq();
}

/* fifo解锁函数 */
static void fifo_unlock(void)
{
  __enable_irq();
}


uint8_t User_Network_Get_Bin(uint8_t *IP, uint8_t *Bin_Path, uint8_t SSLEN)
{

  /*注册FIFO*/
  fifo_register(&Http_RX_fifo, &Http_Buffer[0], sizeof(Http_Buffer), fifo_lock, fifo_unlock);

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

  /*设置连接ID和数据长度*/
  Bsp_ESP8266_Config(AT_Buff, AT_Len, "OK", ">", 30, 3);

  /*设置串口功能*/
  User_UART_RX_Fun = User_Network_Down_Flash;
  
  /*发送Get请求到服务器*/
  Bsp_ESP8266_TX(tcp_buff, tcp_buff_Len);

  while(Flash_Finished != 1)
  {
    User_UART_RX_Loop();
  }


  return 0;
}




//uint8_t Data_Buffer[512] = {0};

void User_Network_Down_Flash(uint8_t *Data , uint16_t Len)
{
  static uint8_t Http_End_Flag = 0;

  if(Http_End_Flag == 0)
  {
    char *p;
    char http_End[] = "Accept-Ranges: bytes\r\n\r\n";
  
    p = strstr((char *)Data, http_End);

    if( p != NULL)
    {
      for(uint16_t i = 0 ; i < Len ; i++)
      {
        if(Data[i + 1] == 'A' && Data[i + 2] == 'c' && Data[i +3] == 'c' && Data[i +4] == 'e' && Data[i +5] == 'p'&& Data[i +6] == 't'&& Data[i + 7] == '-' && Data[i +8] == 'R' )
        {
          if(Data[i + 9] == 'a' && Data[i + 10] == 'n' && Data[i +11] == 'g' && Data[i +12] == 'e' && Data[i +13] == 's'&& Data[i +14] == ':'&& Data[i + 15] ==  ' ')
          {
            if(Data[i + 16] == 'b' && Data[i + 17] == 'y' && Data[i +18] == 't' && Data[i +19] == 'e' && Data[i +20] == 's'&& Data[i +21] == 0x0d && Data[i + 22] ==  0x0a)
            {
              uint16_t Bin_Number = Len - i - 1 - strlen(http_End);
              uint32_t Offset_Add = i + strlen(http_End) + 1;
              //memcpy((char *)Buffer , Data + i + strlen(http_End) + 1, bin_Number);

              /*写入FIFO*/
              fifo_write(&Http_RX_fifo , Data + Offset_Add , Bin_Number);
              Http_End_Flag = 1;
              break;
            }
          }
        }
      }
    }
  }
  else
  {
    fifo_write(&Http_RX_fifo , Data , Len);
  }
  
  uint16_t occupy_size = fifo_get_occupy_size(&Http_RX_fifo);

  if(occupy_size % 32 == 0 && occupy_size != 0)
  {
    uint8_t temp[1024];
    uint16_t len = fifo_read(&Http_RX_fifo , temp , 1024);

    User_App_MCU_Flash_Updata(temp , len);
  }

  //User_UART_RX_Fun = User_App_MCU_Flash_Updata;
  
  
}




/*
 * @Description:
 * @Autor: Pi
 * @Date: 2022-07-06 21:19:14
 * @LastEditTime: 2022-07-12 20:00:25
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

  Bsp_ESP8266_Reset(); //恢复出厂设置
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
    /*解析数据*/
    User_Network_Info_Handle(NetWork_Buff, NETWORK_BUFF_LEN);
  }

  return 0;
}


/**
 * @brief 版本信息数据处理
 * @param {uint8_t} *data
 * @param {uint8_t} len
 * @return {*}
 */
uint8_t User_Network_Info_Handle(uint8_t *data, uint16_t len)
{
  /*当前版本号*/
  uint8_t Version[20];
  uint8_t buff[255];

  char *str;

  //获取版本号
  str = StrBetwString((char *)data, "\"version\":\"", "\",\"");

  //获取版本号长度
  len = strlen(str); 

  /*版本号错误*/
  if (str == NULL || len > 20)
  {
    return 1;
  }
  
 
  //版本号不一样
  if (memcmp(str, Version, len) != 0)
  {
    cStringRestore();

    //获取下载地址 URL
    str = StrBetwString((char *)data, "url\":\"", "\",\"");

    // URL 缓存到数组
    memset(buff, 0, sizeof(buff));
    sprintf((char *)buff, "%s", str);

    if (User_Network_Resolve_Url(buff , &System_infor) == 0)
    {
      //User_Boot_Infor_Set(System_infor);
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







/**
 * @brief URL解析
 * @param {uint8_t} *ch   数据
 * @param {App_information_Str} *Infor    
 * @return {uint8_t} 0:成功     其他:错误
 */
uint8_t User_Network_Resolve_Url(uint8_t *ch , App_information_Str *Infor)
{

  if (ch == NULL || (strlen((char *)ch) < 5))
  {
    return 1;
  }

  uint8_t Len = 0;

  // http or https
  if (memcmp(ch, "https", 5) == 0)
  {
    Infor->SSLEN = 1;
    Len = 5;
  }
  else if (memcmp(ch, "http", 4) == 0)
  {
    Infor->SSLEN = 0;
    Len = 4;
  }
  else
  {
    return 2;
  }

  /*IP*/
  uint8_t *Buffer = (uint8_t *)StrBetwString((char *)ch + Len, "://", ":");

  if (Buffer != NULL)
  {
    //带端口号
    memset(Infor->IP, 0, sizeof(Infor->IP));
    memcpy(Infor->IP, Buffer, strlen((char *)Buffer));

    Len = Len + 3 + strlen((char *)Infor->IP);

    cStringRestore();

    Buffer = (uint8_t *)StrBetwString((char *)ch + Len, ":", "/");   // Port

    if (Buffer != NULL)
    {
      if (strlen((char *)Buffer) < 6 && atoi((char *)Buffer) != 0)
      {
        Infor->Port = atoi((char *)Buffer);
        Len = Len + 1 + strlen((char *)Buffer); // 18/19

        cStringRestore();
        // Path
        memcpy(Infor->Bin_Path, ch + Len, sizeof(Infor->Bin_Path) - Len);
      }
      else
      {
        return 3;
      }
    }
    else
    {
      cStringRestore();
      return 4;
    }

    return 0;
  }

  //不带端口号
  cStringRestore();

  if (Infor->SSLEN)
  {
    Infor->Port = 443;
  }
  else
  {
    Infor->Port = 80;
  }

  // IP
  Buffer = (uint8_t *)StrBetwString((char *)ch + Len, "://", "/");
  if (Buffer != NULL)
  {
    memset(Infor->IP, 0, sizeof(Infor->IP));
    memcpy(Infor->IP, Buffer, strlen((char *)Buffer));
    Len = Len + 3 + strlen((char *)Infor->IP); // 14/15
  }
  else
  {
    return 6;
  }
  cStringRestore();
  // Path
  memcpy(Infor->Bin_Path, ch + Len, sizeof(Infor->Bin_Path) - Len);


  return 0;
}

/*
 * @Description:
 * @Autor: Pi
 * @Date: 2022-07-06 21:19:14
 * @LastEditTime: 2022-07-21 19:56:11
 */
#include "network.h"

/*HAL库句柄*/
extern TIM_HandleTypeDef htim12;

/*内部使用函数*/
static void User_Network_Url_Process(uint8_t *pStr, Info_Str *Info);
static Info_Str User_Network_Info_Process(uint8_t *data, uint16_t len);

static void User_Network_Finished(uint8_t *data, uint16_t len); //串口接收数据处理完成函数指针
static void User_Network_RX_Fun(uint8_t *data, uint16_t len);   //串口接收数据处理函数指针

struct
{
  uint16_t Count;
  uint16_t Max;
  uint8_t Flag;
  uint8_t Enable;
} Network_Timer = {0, 20, 0, 0};

/**
 * @brief 设置默认连接路由器
 * @param {uint8_t} *SSID   连接的路由器名称
 * @param {uint8_t} *PAW    连接的路由器密码
 * @return {*}
 */
uint8_t User_Network_Connect_AP(uint8_t *SSID, uint8_t *PAW)
{
  if (Bsp_ESP8266_Connect_AP(SSID, PAW) == 0)
  {
    return 0;
  }
  else
  {
    return 1;
  }
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
  if (Bsp_ESP8266_Connect_TCP(IP, Port, Https_Enable) == 0)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

uint8_t Finish_Flag = 0;
uint8_t Info_Bufer[1024];
uint16_t Info_Pos = 0;
/**
 * @brief 发送Get请求获取版本信息
 * @param {uint8_t} *IP
 * @param {uint8_t} *Bin_Path
 * @param {uint8_t} SSLEN
 * @param {uint8_t} Info
 * @return {*}
 */
uint8_t User_Network_Get_Info(uint8_t *IP, uint8_t *Info_Path, uint8_t SSLEN, Info_Str *Info)
{

  /*发送失败*/
  if (Bsp_ESP8266_Send_Get_Request(IP, Info_Path, SSLEN) == 1)
  {
    return 1;
  }

  User_UART_RX_Fun = User_Network_RX_Fun;
  User_UART_RX_Finished = User_Network_Finished;

  do
  {
    User_UART_RX_Loop();
  } while (Finish_Flag != 1);

  /*关闭连接*/
  // Bsp_ESP8266_Config("AT+CIPCLOSE=0\r\n", 16, "0,CLOSED", NULL, 10, 3); //非透传模式

  *Info = User_Network_Info_Process(Info_Bufer, sizeof(Info_Bufer));

  return 0;
}

void User_Network_RX_Fun(uint8_t *data, uint16_t len) //串口接收数据处理函数指针
{
  memcpy(Info_Bufer + Info_Pos, data, len);
  Info_Pos += len;
}

void User_Network_Finished(uint8_t *data, uint16_t len) //串口接收数据处理完成函数指针
{
  memcpy(Info_Bufer + Info_Pos, data, len);
  Info_Pos += len;
  Finish_Flag = 1;
}

/**
 * @brief Inof文本数据解析
 * @param {uint8_t} *data   Info数据
 * @param {uint16_t} len    数据长度
 * @return {*}
 */
static Info_Str User_Network_Info_Process(uint8_t *data, uint16_t len)
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



#define Bin_Buffer_Len 1024
uint8_t Bin_Buffer[Bin_Buffer_Len];
uint16_t Bin_Buufer_Count = 0;


void User_Network_Get_Bin(uint8_t *IP, uint8_t *Bin_Path, uint8_t SSLEN)
{

  if (Bsp_ESP8266_Send_Get_Request(IP, Bin_Path, SSLEN) == 1)
  {
    return;
  }

  uint8_t Temp_Data;


  /*循环找到HTTP头部*/
  while(1)
  {
    uint8_t Data = 0;
    uint16_t Occup_Size = Bsp_UART_Get_RX_Buff_Occupy(&huart1);

    if (Occup_Size > 0)
    {
      Bsp_UART_Read(&huart1, &Temp_Data, 1);

      if(User_Networt_IPD_Process(Temp_Data , &Data) == 0)
      {
        if(User_Networt_HTTP_Process(Data) == 0)
        {
          break;
        }
      }
    }
  }

  /*数据部分*/
  while (1)
  {
    uint8_t Data = 0;
    uint16_t Occup_Size = Bsp_UART_Get_RX_Buff_Occupy(&huart1);

    if (Occup_Size > 0)
    {
      if(Network_Timer.Enable == 1)
      {
        User_Networt_Timer_Enable(0);
      }

      Bsp_UART_Read(&huart1, &Temp_Data, 1);

      if(User_Networt_IPD_Process(Temp_Data , &Data) == 0)
      {
        Bin_Buffer[Bin_Buufer_Count] = Data;
        Bin_Buufer_Count++;

        if(Bin_Buufer_Count == Bin_Buffer_Len)
        {
          Bin_Buufer_Count = 0;
          User_App_MCU_Flash_Updata(Bin_Buffer , Bin_Buffer_Len);
          memset(Bin_Buffer , 0 , Bin_Buffer_Len);
        }
      }
    }
    else
    {
      /*打开定时器*/
      if(Network_Timer.Enable == 0)
      {
        User_Networt_Timer_Enable(1);
      }
      
      /*定时器超时*/
      if(Network_Timer.Flag == 1)
      {
        Network_Timer.Flag = 0;

        if(Bin_Buufer_Count > 0)
        {
          Bin_Buufer_Count = 0;
          User_App_MCU_Flash_Updata(Bin_Buffer , Bin_Buffer_Len);
          memset(Bin_Buffer , 0 , Bin_Buffer_Len);
        }

        return 0;
      }

    }


  }
  
    
}





struct 
{
  uint8_t Count;
  uint8_t ID;
  uint8_t Len_Count;
}IPD;



struct  
{
  uint16_t Data_Len;
  uint8_t  Start;     
  uint8_t  Flag;     //是否是网络数据
}Recv; 










/*功能已实现 待重构优化*/

static uint8_t IPD_Count = 0;
static uint8_t IPD_ID = 0;

static uint16_t Recv_Data_Len = 0;
static uint8_t Recv_Start = 0;
static uint8_t Recv_Flag = 0;     //是否是网络数据
static uint8_t Recv_Count = 0;    //接收到的数据长度 计数




uint8_t User_Networt_IPD_Process(uint8_t data , uint8_t *return_data)
{
  uint8_t Ret = 1;
  
  /*返回网络数据*/
  if(Recv_Flag == 1)
  {
    if(Recv_Data_Len > 0)    //还没结束
    {
      Recv_Data_Len--;
      *return_data = data;
      Ret = 0;
    }
    else      //数据结束
    {
      Recv_Flag = 0;
      Recv_Data_Len = 0;
      Recv_Start = 0;
    }
  }


  /*接收到的数据个数*/
  if(Recv_Start == 1)
  {
    if(Recv_Count <=5)
    {
      Recv_Count++;

      if(data == ':')     //接收到数据长度
      {
        Recv_Start = 0;
        Recv_Count = 0;
        Recv_Flag = 1;
      }
      else
      {
        if(data >= '0' && data <= '9')      //取出数据长度
        {
          Recv_Data_Len = Recv_Data_Len * 10;
          Recv_Data_Len = Recv_Data_Len + (data -0x30);
        }
      }
    }
    else
    {
      Recv_Count = 0;
    }

  }

  if(data == '+' && IPD_Count == 0)IPD_Count++;
  else if(data == 'I' && IPD_Count == 1)IPD_Count++;
  else if(data == 'P' && IPD_Count == 2)IPD_Count++;
  else if(data == 'D' && IPD_Count == 3)IPD_Count++;
  else if(data == ',' && IPD_Count == 4)IPD_Count++;
  else if(data >= '0'	&& data <='9' && IPD_Count == 5){IPD_Count++;IPD_ID = data;}        /*自行决定是否需要返回ID号*/
  else if(data == ',' && IPD_Count == 6){IPD_Count = 0 , Recv_Start = 1;  Recv_Count = 0; Recv_Data_Len = 0;}
  else
  {
    if(data == '+' && IPD_Count == 1)
    {
      IPD_Count = 1;
    }
    else
    {
      IPD_Count = 0;
    }
  }

  return Ret;
}




/**
 * @brief 判断是否收到HTTP消息头 需要放在最前面进行解析
 * @param {uint8_t} data    单个数据
 * @return {uint8_t}  0:成功  1:失败
 */
uint8_t User_Networt_HTTP_Process(uint8_t data)
{
  uint8_t http_End[] = "Accept-Ranges: bytes\r\n\r\n";

  static uint8_t RX_Count = 0;

  /*HTTP头消息未收到*/
  if (http_End[RX_Count] == data)
  {
    RX_Count++;
    if (RX_Count >= 24)
    {
      RX_Count = 0;
      return 0;
    }
  }
  
  return 1;
}





/**
 * @brief 超时服务函数 放中断
 * @return {*}
 */
void User_Networt_Timer(void)
{
  Network_Timer.Count++;

  if (Network_Timer.Count >= Network_Timer.Max)
  {
    Network_Timer.Count = 0;
    Network_Timer.Flag = 1;
    HAL_TIM_Base_Stop_IT(&htim12);
    __HAL_TIM_SET_COUNTER(&htim12, 0);
  }
}

/**
 * @brief 定时器使能
 * @param {uint8_t} Enable
 * @return {*}
 */
void User_Networt_Timer_Enable(uint8_t Enable)
{
  if (Enable)
  {
    Network_Timer.Count = 0;
    Network_Timer.Enable = 1;
    __HAL_TIM_CLEAR_FLAG(&htim12, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(&htim12);
  }
  else
  {
    Network_Timer.Enable = 0;
    Network_Timer.Count = 0;
    HAL_TIM_Base_Stop_IT(&htim12);
    __HAL_TIM_SET_COUNTER(&htim12, 0);
  }
}

/*
 * @Description:
 * @Autor: Pi
 * @Date: 2022-07-06 21:19:14
 * @LastEditTime: 2022-07-20 22:11:58
 */
#include "network.h"

/*HAL库句柄*/
extern TIM_HandleTypeDef htim12;


/*内部使用函数*/
static void User_Network_Url_Process(uint8_t *pStr, Info_Str *Info);
static Info_Str User_Network_Info_Process(uint8_t *data, uint16_t len);

static void User_Network_Finished(uint8_t *data, uint16_t len);                      //串口接收数据处理完成函数指针
static void User_Network_RX_Fun(uint8_t *data, uint16_t len);                           //串口接收数据处理函数指针


struct 
{
  uint16_t Max; // 100ms * 30 = 3S
  uint16_t Count;
  uint8_t  Flag;
}Network_Timer;

/*目标回复结构体*/
/*
struct
{
  uint8_t Flag;
  uint8_t *Target0;
  uint8_t *Target1;
} Reply_Target;
*/
//#define NETWORK_BUFF_LEN 1024
//uint8_t NetWork_Buff[NETWORK_BUFF_LEN];



/**
 * @brief 设置默认连接路由器
 * @param {uint8_t} *SSID   连接的路由器名称
 * @param {uint8_t} *PAW    连接的路由器密码
 * @return {*}
 */
uint8_t User_Network_Connect_AP(uint8_t *SSID, uint8_t *PAW)
{
  if(Bsp_ESP8266_Connect_AP(SSID , PAW) == 0)
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
  if(Bsp_ESP8266_Connect_TCP(IP , Port , Https_Enable) == 0)
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
uint8_t User_Network_Get_Info(uint8_t *IP, uint8_t *Info_Path, uint8_t SSLEN , Info_Str *Info)
{

  /*发送失败*/
  if(Bsp_ESP8266_Send_Get_Request(IP , Info_Path , SSLEN) == 1)
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
  //Bsp_ESP8266_Config("AT+CIPCLOSE=0\r\n", 16, "0,CLOSED", NULL, 10, 3); //非透传模式

  *Info = User_Network_Info_Process(Info_Bufer , sizeof(Info_Bufer));

  return 0;
  
}


 
void User_Network_RX_Fun(uint8_t *data, uint16_t len)                           //串口接收数据处理函数指针
{
  memcpy(Info_Bufer+Info_Pos , data , len);
  Info_Pos += len;
}

void User_Network_Finished(uint8_t *data, uint16_t len)                      //串口接收数据处理完成函数指针
{
  memcpy(Info_Bufer+Info_Pos , data , len);
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

#define Bin_Buffer_Len 128
uint8_t Bin_Buffer[Bin_Buffer_Len];
uint8_t Updata_Flag = 0;
uint16_t Updata_Tick = 0;
uint32_t Updata_Size = 0;
uint8_t IPD[] = "+IPD,0";

uint8_t User_Network_Get_Bin(uint8_t *IP, uint8_t *Bin_Path, uint8_t SSLEN)
{
  
  /*发送失败*/
  if(Bsp_ESP8266_Send_Get_Request(IP , Bin_Path , SSLEN) == 1)
  {
    return 1;
  }

  static uint8_t Http_End_Flag = 0;
  static char http_End[] = "Accept-Ranges: bytes\r\n\r\n";  
  static uint8_t RX_Count = 0;

  while(1)
  {
    uint16_t size = Bsp_UART_Get_RX_Buff_Occupy(&huart1);
    
    if(Http_End_Flag == 0)
    {
      if(size > 0)
      {
        uint8_t Temp;
        Bsp_UART_Read(&huart1 , &Temp , 1);

        if( http_End[RX_Count] == Temp )
        {
          RX_Count++;
          if(RX_Count >= 24)
          {
            RX_Count = 0;
            Http_End_Flag = 1;
          }
        }
      }
    }
    else
    {
      if(size >= Bin_Buffer_Len)
      {
        Updata_Tick = 0;
        Updata_Flag = 0;
        HAL_TIM_Base_Stop_IT(&htim12);
        __HAL_TIM_SET_COUNTER(&htim12 , 0);
        uint16_t Tem = Bsp_UART_Read(&huart1 , Bin_Buffer , Bin_Buffer_Len);

        
        User_App_MCU_Flash_Updata(Bin_Buffer , Bin_Buffer_Len);
        Updata_Size += Tem;
      }
      else
      {
        if(Updata_Flag == 0)
        {
          __HAL_TIM_CLEAR_FLAG(&htim12 , TIM_FLAG_UPDATE);
          HAL_TIM_Base_Start_IT(&htim12);
        }
        else
        {
          uint16_t Tem = Bsp_UART_Read(&huart1 , Bin_Buffer , Bin_Buffer_Len);
          User_App_MCU_Flash_Updata(Bin_Buffer , Bin_Buffer_Len);

          Updata_Size += Tem;
          return 0;
        }
      }
    }

  }

  /*
  User_Uart_RX_Timeout_Set(1000);
  User_UART_RX_Fun = User_App_MCU_Flash_Updata;
  User_UART_RX_Finished = User_Network_Finished;

  do
  {
    User_UART_RX_Loop();
  } while (Flash_Finished != 1);
  */

  /*关闭连接*/
  //Bsp_ESP8266_Config("AT+CIPCLOSE=0\r\n", 16, "0,CLOSED", NULL, 10, 3); //非透传模式
  
  




}




 

void User_Network_Down_Flash(uint8_t *Data , uint16_t Len)
{
  static uint8_t Http_End_Flag = 0;
  static char http_End[] = "Accept-Ranges: bytes\r\n\r\n";  
  static uint8_t RX_Count = 0;

  if(Http_End_Flag == 0)
  {
    if( http_End[RX_Count] == Data[0] )
    {

      RX_Count++;
      if(RX_Count >= 24)
      {
        RX_Count = 0;
        Http_End_Flag = 1;
        //User_UART_RX_Read_Len(0);
      }
    }
  }
  else
  {
    User_UART_RX_Fun = User_App_MCU_Flash_Updata;
  }

  
}




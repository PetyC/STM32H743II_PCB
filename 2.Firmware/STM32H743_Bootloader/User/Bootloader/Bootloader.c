/*
 * @Description: Bootloader跳转到APP程序
 * @Autor: Pi
 * @Date: 2022-07-01 16:53:36
 * @LastEditTime: 2022-07-05 19:57:20
 */
#include "Bootloader.h"


#include <stdio.h>

/*系统状态存储*/
uint32_t APP_Updata_Flag __attribute__((at(0x20000000), zero_init));
SYS_State_Enum System_State;        //系统状态
uint8_t UART_RX_Time_Out_Flag = 0; //串口超时标志
uint32_t APP_Bin_Size = 0;         // APP Bin文件大小

/*内部调用*/
static void Jump_To_App(void);

/*相关句柄*/
extern TIM_HandleTypeDef htim13;

/**
 * @brief 跳转到APP应用
 * @return {*}
 */
static void Jump_To_App(void)
{
  /* 声明一个函数指针 */
  void (*AppJump)(void);

  /* 跳转到应用程序，首地址是MSP，地址+4是复位中断服务程序地址 */
  AppJump = (void (*)(void))(*((uint32_t *)(MCU_FLASH_APP_ADDR + 4)));

  /* 设置主堆栈指针 */
  __set_MSP(*(uint32_t *)MCU_FLASH_APP_ADDR);

  /* 跳转到系统BootLoader */
  AppJump();

  /* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
  while (1)
  {
  }
}

/**
 * @brief 擦除APP占用扇区
 * @param {uint32_t} APP_File_Size    APP Bin文件大小(单位 字节)
 * @return {uint8_t} 0:成功  1:失败
 */
uint8_t User_MCU_Flash_APP_Erase(uint32_t APP_File_Size)
{
  /*APP Bin文件占用几个整数扇区*/
  uint32_t APP_File_Sector_Count = APP_File_Size / (128 * 1024);

  /*APP Bin文件占用整数扇区剩余数*/
  uint32_t APP_File_Sector_Remain = APP_File_Size % (128 * 1024);

  if (APP_File_Sector_Remain != 0)
  {
    APP_File_Sector_Count++;
  }

  /*擦除APP占有扇区*/
  if (User_MCU_Flash_Erase(MCU_FLASH_APP_ADDR, APP_File_Sector_Count) == 0)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

/**
 * @brief 从串口接收APP数据 并写入FALSH中
 * @param {uint8_t} *Updata_Finish     APP更新完成标志 0:未完成  1:完成
 * @return {uint8_t}0:成功   1:失败
 */
uint8_t User_Update_Flash_APP(uint8_t *Updata_Finish)
{
  /*APP更新完成标志*/
  *Updata_Finish = 0;

  /*Flash写入是否出错*/
  static bool Flash_Error = 0;

  /*最少写入Flash数据量*/
  uint16_t RX_Buff_MAX = 512;

  /*APP写入偏移地址*/
  static uint32_t APP_Temp_ADDR = 0;

  /*获得当前RX 缓存数*/
  uint16_t Buff_Occupy = User_UART_Get_RX_Buff_Occupy(&huart1);

  if (Buff_Occupy < RX_Buff_MAX && Buff_Occupy > 0)
  {
    //串口超时定时器开启
    if (UART_RX_Time_Out_Flag == 0)
    {
      HAL_TIM_Base_Start_IT(&htim13);
    }
    //定时器超时,则数据接收结束
    if (UART_RX_Time_Out_Flag == 1)
    {
      UART_RX_Time_Out_Flag = 0;

      uint8_t Uart_Data[512];
      uint16_t size = User_UART_Read(&huart1, Uart_Data, sizeof(Uart_Data));
      Flash_Error = User_MCU_Flash_Write(MCU_FLASH_APP_ADDR + APP_Temp_ADDR, Uart_Data, size);
      *Updata_Finish = 1;
    }
  }
  else if (Buff_Occupy >= RX_Buff_MAX)
  {
    uint8_t Uart_Data[512];
    User_UART_Read(&huart1, Uart_Data, sizeof(Uart_Data));
    Flash_Error = User_MCU_Flash_Write(MCU_FLASH_APP_ADDR + APP_Temp_ADDR, Uart_Data, sizeof(Uart_Data));

    APP_Temp_ADDR += sizeof(Uart_Data);

    if (UART_RX_Time_Out_Flag == 1)
    {
      UART_RX_Time_Out_Flag = 0;
      HAL_TIM_Base_Stop(&htim13);
    }
  }

  return Flash_Error;
}


#define APP_SIZE (uint32_t)(128 * 7 * 1024)     //单位：字节
/*内部程序复制到W25Q*/
void Demo()
{
  uint8_t Data[256];

  for(uint32_t i = 0 ; i < APP_SIZE/256 ; i++)
  {
    User_MCU_FLASH_Read(MCU_FLASH_APP_ADDR + (i * 256), Data , sizeof(Data));      //读取一页256字节

    QSPI_W25Qx_Write_Buffer(FLASH_BEGIN_ADDRESS + (i * 256) , Data , sizeof(Data)); //写入外置Flash
  }


  if(APP_SIZE%256 != 0)
  {
    memset(Data , 0 , 256);
    User_MCU_FLASH_Read(MCU_FLASH_APP_ADDR + ((APP_SIZE/256 + 1)  * 256) , Data , sizeof(Data));      //读取一页256字节
    QSPI_W25Qx_Write_Buffer(FLASH_BEGIN_ADDRESS + ((APP_SIZE/256 + 1)  * 256)  , Data , sizeof(Data)); //写入外置Flash

  }
  
}


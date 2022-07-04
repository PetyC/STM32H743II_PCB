/*
 * @Description: Bootloader跳转到APP程序
 * @Autor: Pi
 * @Date: 2022-07-01 16:53:36
 * @LastEditTime: 2022-07-05 01:21:13
 */
#include "Bootloader.h"
#include "Bsp_MCU_Flash.h"
#include "Bsp_Uart.h"

#include "st7735s.h"
#include "fonts.h"
#include "gfx.h"

#include <stdio.h>

/*系统状态存储*/
uint32_t System_State __attribute__((at(0x20000000), zero_init));
uint8_t UART_RX_Time_Out_Flag = 0; //串口超时标志

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






/**
 * @brief
 * @return {*}
 */
void Bootloader_Loop(void)
{
  static uint8_t Updata_Error_Flag = 0;
  static uint8_t Updata_Finish_Flag = 0;
  static uint8_t Flash_Erase_Error = 0;

  /*APP Bin文件大小*/
  //uint32_t APP_Bin_Size = 70628;

  switch (System_State)
  {
  case Jump_APP:        //跳入APP
    NVIC_SystemReset(); //复位CPU
    break;

  case Update_APP: //需要升级APP


    System_State = Erase_Flash_APP;
    break;

  case Erase_Flash_APP:

    /*擦除APP FLASH*/
    Flash_Erase_Error = User_MCU_Flash_APP_Erase(70628);

    /*擦除Flash出错*/
    if (Flash_Erase_Error == 1)
    {
      System_State = Update_Error_APP;
      Updata_Error_Flag = 1;

      break;
    }

    System_State = Write_Flash_APP;

    break;

  case Write_Flash_APP:

    Updata_Error_Flag = User_Update_Flash_APP(&Updata_Finish_Flag);

    /*升级APP出错*/
    if (Updata_Error_Flag == 1)
    {
      System_State = Update_Error_APP;
      break;
    }

    /*升级APP完成*/
    if (Updata_Finish_Flag == 1)
    {
      System_State = Updata_Finish;
    }

    break;
  case Updata_Finish:

    /*UI 跳转倒计时*/
    drawText(13 * 6, 12 * 5, "OK     ");


    System_State = Jump_APP;
    
    NVIC_SystemReset(); //复位CPU

    break;
  case Update_Error_APP: //升级APP出错

    break;

  default:
    break;
  }
}





void Demo()
{

  switch (System_State)
  {
    case Update_APP://需要升级APP

    System_State = Erase_Flash_APP;
    break;

    case Erase_Flash_APP://擦除APP Flash

    System_State = Write_Flash_APP;
    break;

    case Write_Flash_APP://APP写入Flash

    System_State = Updata_Finish;
    break;
  
    
    case Updata_Finish://APP升级完成


    System_State = Jump_APP;
    break;

    case Update_Error_APP://APP错误

        
    break;

    case Jump_APP://跳入APP


    break;
    

    default:
    break;

  }



}
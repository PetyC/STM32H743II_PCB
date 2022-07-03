/*
 * @Description: Bootloader跳转到APP程序
 * @Autor: Pi
 * @Date: 2022-07-01 16:53:36
 * @LastEditTime: 2022-07-03 23:21:17
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
 * @brief  判断是否软件复位后再进入APP，提供一个干净的CPU环境给APP
 * @return {*}
 */
void Judge_Jump_APP(void)
{
  if ((SYS_State_Enum)System_State == Jump_APP)
  {
    /*跳入APP*/
    Jump_To_App();
  }
  else
  {
    return;
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

bool UART_RX_Time_Out_Flag = 0; //串口超时标志

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
 * @brief 定时器中断回调
 * @param {TIM_HandleTypeDef} *htim
 * @return {*}
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM13)
  {
    HAL_TIM_Base_Stop(&htim13);
    UART_RX_Time_Out_Flag = 1;
  }
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
	uint32_t APP_Bin_Size = 70628;
	
  switch (System_State)
  {
  case Jump_APP:        //跳入APP
    NVIC_SystemReset(); //复位CPU
    break;

  case Update_APP: //需要升级APP

    setColor(0, 0xff, 00);
    setbgColor(0, 0, 0);
    setFont(ter_u12b);
    drawText(0, 12 + 2, "System:Update_APP");

    drawText(0, (12 ) * 2, "APP Addr:");
    drawText(0, (12 ) * 3, "APP Size:");
    flushBuffer();
    drawText(0, (12) * 4, "Flash Erase:");
    drawText(0, (12) * 5, "Flash Write:");
    drawText(0, (12) * 6, "Jump APP:");
    flushBuffer();
    /*获取APP文件大小和起始地址*/
    char Temp[30];
    sprintf(Temp , "0X%x" , MCU_FLASH_APP_ADDR);
    drawText(10 * 6, (12) * 2, Temp);

    memset(Temp , 0 , sizeof(Temp));

    sprintf(Temp , "%d" , APP_Bin_Size);
    drawText(10 * 6, (12) * 3, Temp);


    flushBuffer();

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
      drawText(13 * 6, 12 * 4, "Error");
      flushBuffer();
      break;
    }

    drawText(13 * 6, 12 * 4, "OK");
    flushBuffer();

    System_State = Write_Flash_APP;

    break;

  case Write_Flash_APP:

    Updata_Error_Flag = User_Update_Flash_APP(&Updata_Finish_Flag);

    /*升级APP出错*/
    if (Updata_Error_Flag == 1)
    {
      System_State = Update_Error_APP;
      drawText(13 * 6, 12 * 5, "Error");
      flushBuffer();
      break;
    }

    /*升级APP完成*/
    if (Updata_Finish_Flag == 1)
    {
      /*UI 跳转倒计时*/
      drawText(13 * 6, 12 * 5, "OK");
      for(uint8_t i = 5 ; i > 0 ; i--)
      {
        char Number;
        sprintf(&Number , "%d" , i);
        drawText(10 * 6, 12 * 6, &Number);
        flushBuffer();
        HAL_Delay(1000);
      }

      System_State = Jump_APP;
      NVIC_SystemReset(); //复位CPU
    }
    
    break;

  case Update_Error_APP: //升级APP出错

    break;

  default:
    break;
  }
}

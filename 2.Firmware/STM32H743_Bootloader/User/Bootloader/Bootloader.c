/*
 * @Description: Bootloader跳转到APP程序
 * @Autor: Pi
 * @Date: 2022-07-01 16:53:36
 * @LastEditTime: 2022-07-08 18:31:30
 */
#include "Bootloader.h"

#include <stdio.h>

/*APP跳转标志*/
uint32_t APP_Jump_Flag __attribute__((at(0x20000000), zero_init));

/*APP文件大小*/
uint32_t App_Updata_Size;

/*串口超时标志*/
uint8_t UART_RX_Time_Out_Flag = 0;

/*信息存储*/
App_information_Str System_infor;

/*内部调用*/

/*相关句柄*/
extern TIM_HandleTypeDef htim13;
extern CRC_HandleTypeDef hcrc;

/**
 * @brief 跳转到APP应用
 * @return {*}
 */
void User_App_Jump(void)
{
  /*判断是否跳转*/
  if (APP_Jump_Flag != APP_JUMP_VALUE)
  {
    return;
  }

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
uint8_t User_App_MCU_Flash_Erase(uint32_t APP_File_Size)
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
 * @brief 从串口接收APP数据 并写入内部FALSH中
 * @param {uint8_t} *Updata_Finish     APP更新完成标志 0:未完成  1:完成
 * @return {uint8_t}0:成功   1:失败
 */
uint8_t User_App_MCU_Flash_Updata(uint8_t *Updata_Finish)
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
 * @brief 将APP备份至外部Flash中
 * @return {uint8_t}0:成功    1:失败
 */
uint8_t User_App_MCU_Flash_Copy(void)
{
  uint8_t Data[256];

  /*擦除外置FLASH APP备份区 2MB*/
  for (uint32_t i = 0; i < (FLASH_BIN_SIZE / 4096); i++)
  {
    QSPI_W25Qx_EraseSector(FLASH_BIN_ADDR + (i * 4096));
  }

  /*将MCU内部APP备份到外置FLASH上*/
  for (uint32_t i = 0; i < MCU_FLASH_USER_SIZE / 256; i++)
  {
    if (User_MCU_FLASH_Read(MCU_FLASH_APP_ADDR + (i * 256), Data, sizeof(Data)) == 0) //读取一页256字节
    {
      QSPI_W25Qx_Write_Buffer(FLASH_BIN_ADDR + (i * 256), Data, sizeof(Data)); //写入外置Flash
    }
    else
    {
      return 1;
    }
  }
  return 0;
}

/**
 * @brief 将外部Flash备份数据写入内部Flash
 * @return {*}
 */
uint8_t User_App_Flash_Copy(void)
{
  uint8_t Data[512];
  uint8_t Flash_Error = 0;

  /*将外部Flash数据写入内部FLASH*/
  for (uint32_t i = 0; i < FLASH_BIN_SIZE / 512; i++)
  {
    QSPI_W25Qx_Read_Buffer(FLASH_BIN_ADDR + (i * 512), Data, sizeof(Data));

    Flash_Error = User_MCU_Flash_Write(MCU_FLASH_APP_ADDR + (i * 512), Data, sizeof(Data));

    if (Flash_Error == 1)
    {
      return 1;
    }
  }

  return 0;
}

/**
 * @brief CRC校验写入FLASH数据
 * @param {uint32_t} APP_File_Size: app bin文件的大小 单位:字节
 * @return {uint8_t} 0:正确  1:错误
 */
uint8_t User_App_MCU_Flash_CRC(uint32_t APP_File_Size)
{
  __IO uint32_t Bin_CRC_Value;
  __IO uint32_t Ret_CRC_Value;

  /* 读取bin文件的CRC */
  Bin_CRC_Value = *(__IO uint32_t *)(MCU_FLASH_APP_ADDR + APP_File_Size - 4);

  /* 计算是否与硬件CRC一致 */
  Ret_CRC_Value = HAL_CRC_Calculate(&hcrc, (uint32_t *)MCU_FLASH_APP_ADDR, APP_File_Size / 4 - 1);

  if (Ret_CRC_Value != Bin_CRC_Value)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/**
 * @brief 启动跳转到APP
 * @return {*}
 */
void User_App_Jump_Start(void)
{
  /*设置全局标志*/
  APP_Jump_Flag = APP_JUMP_VALUE;

  /* 复位CPU */
  NVIC_SystemReset();
}




/**
 * @brief Boot初始化，读取外置FLASH判断是否需要升级，若不需要升级则跳入APP
 * @return {*}
 */
void User_Boot_Init(void)
{
  /*读取存储的系统信息*/
  QSPI_W25Qx_Read_Buffer(FLASH_DATA_ADDR, (uint8_t *)&System_infor, sizeof(App_information_Str));

  /*信息未初始化则进行初始化*/
  if (System_infor.Init != 0)
  {
    QSPI_W25Qx_EraseSector(FLASH_DATA_ADDR);

    System_infor.Init = 1;
    System_infor.Version = 0;
    System_infor.Size = 0;
    System_infor.Updata = 0;
    System_infor.SSLEN = 0;
    System_infor.Port = 80;
    memcpy(System_infor.IP, "www.qiandpi.com", 16);
    memcpy(System_infor.Bin_Path , "/ota/hardware/H7-Core/user_crc.bin", 35);
    memcpy(System_infor.Info_Path , "/ota/hardware/H7-Core/info.txt", 31);

    uint16_t Len = sizeof(App_information_Str);
    /*以256字节 写入FLASH*/
    for(uint32_t i = 0 ; i < Len/256 ; i++)
    {
      QSPI_W25Qx_Write_Buffer(FLASH_DATA_ADDR + (i * 256), (uint8_t *)&System_infor + (i*256), 256);
    }
    if(Len % 256 != 0)
    {
      QSPI_W25Qx_Write_Buffer(FLASH_DATA_ADDR + ((Len/256  + 1) * 256), (uint8_t *)&System_infor + (Len/256  + 1) * 256, 256);
    }

  }

  /*无须升级*/
  if (System_infor.Updata == 0)
  {
    //User_App_Jump_Start();
  }
}

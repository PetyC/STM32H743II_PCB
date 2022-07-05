/*
 * @Description: Bootloader跳转到APP程序
 * @Autor: Pi
 * @Date: 2022-07-01 16:53:43
 * @LastEditTime: 2022-07-05 19:51:56
 */
#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "main.h"

#include "Bsp_MCU_Flash.h"
#include "Bsp_Uart.h"
#include "Bsp_w25qxx.h"
#include "st7735s.h"
#include "fonts.h"
#include "gfx.h"


/*用户根据自己的需要设置*/
#define MCU_FLASH_IAP_SIZE (uint32_t)(1024 * 1024) // bootloader所用容量大小(KB)

/*用户程序运行地址:FLASH的起始地址 + bootloader所用程序大小*/
#define MCU_FLASH_APP_ADDR (MCU_FLASH_BASE_ADDR + MCU_FLASH_IAP_SIZE)

/*STM32H743xx 内部一个扇区128KB 不适合单独一个扇区存放数据 */
#if defined(STM32H743xx)
/*用户程序大小(KB) = (总FLASH容量 - bootloader所用容量大小(KB))*/
//#define MCU_FLASH_USER_SIZE (MCU_FLASH_SIZE - MCU_FLASH_IAP_SIZE) 

#define MCU_FLASH_USER_SIZE (uint32_t)(1024 * 128)    //单位:kb

#else
/*存储用户数据所用容量大小(KB)*/
#define MCU_FLASH_DATA_SIZE (uint32_t)(1024 * 2)
/*存储用户数据地址 */
#define MCU_FLASH_DATA_ADDR (MCU_FLASH_BASE_ADDR + (MCU_FLASH_SIZE - MCU_FLASH_DATA_SIZE))
/*用户程序大小(KB) = (总FLASH容量 - 存储用户数据所用容量大小(KB) - bootloader所用容量大小(KB))*/
#define MCU_FLASH_USER_SIZE (MCU_FLASH_SIZE - MCU_FLASH_DATA_SIZE - MCU_FLASH_IAP_SIZE)

#endif



/*系统状态存储*/
extern uint32_t APP_Updata_Flag;          //是否升级标志
extern uint32_t APP_Bin_Size;             //APP Bin文件大小
extern uint8_t UART_RX_Time_Out_Flag;     //串口超时标志


/*系统状态*/
typedef enum 
{
  Check_APP,                //检查是否需要升级APP
  Update_APP,               //需要升级APP
  Get_Size_APP,             //获取Bin文件大小
  Erase_Flash_APP,          //擦除原APP Flash
  Wait_Flash_APP,           //等待数据准备写入FLASH
  Write_Flash_APP,          //APP写入FLASH
  Updata_Finish,            //APP升级完成
  Update_Error_APP,         //升级APP出错
  Jump_APP,                 //跳入APP
}SYS_State_Enum;






uint8_t User_MCU_Flash_APP_Erase(uint32_t APP_File_Size);
uint8_t User_Update_Flash_APP(uint8_t *Updata_Finish);



#endif

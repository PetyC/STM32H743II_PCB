/*
 * @Description: Bootloader跳转到APP程序
 * @Autor: Pi
 * @Date: 2022-07-01 16:53:43
 * @LastEditTime: 2022-07-11 19:50:21
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


/*内部Flash*/
/*bootloader所用容量大小(KB)*/
#define MCU_FLASH_IAP_SIZE (uint32_t)(1024 * 1024) 

/*app 用户程序运行地址:FLASH的起始地址 + bootloader所用程序大小*/
#define MCU_FLASH_APP_ADDR (MCU_FLASH_BASE_ADDR + MCU_FLASH_IAP_SIZE) 

/*app 用户程序大小(KB) = (总FLASH容量 - bootloader所用容量大小(KB))*/
#define MCU_FLASH_USER_SIZE (MCU_FLASH_SIZE - MCU_FLASH_IAP_SIZE) 

/*外部Flash*/
/*用户数据区大小 4096 单位:字节*/
#define FLASH_DATA_SIZE  0X1000                

/*数据区地址*/      
#define FLASH_DATA_ADDR (FLASH_BEGIN_ADDR)          

/*app存储区大小 单位:字节*/
#define FLASH_BIN_SIZE   MCU_FLASH_USER_SIZE                   

/*app区起始地址*/
#define FLASH_BIN_ADDR  (FLASH_DATA_ADDR + FLASH_DATA_SIZE)       

/*需要跳转APP的值*/
#define APP_JUMP_VALUE  0X11223344



/*系统状态存储*/
extern uint32_t APP_Jump_Flag;         //是否升级标志
 
 


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


typedef struct
{
  uint8_t Init;           //是否已初始化
  uint8_t Version;        //目前版本号
  uint8_t Updata;         //是否需要升级
  uint32_t Size;          //固件大小
  uint8_t IP[256];        // 服务器地址or域名
  uint16_t Port;           //端口号
  uint8_t SSLEN;          //1:ssl   0:非SSL
  uint8_t Bin_Path[255];  //Bin文件地址
  uint8_t Info_Path[255];  //配置信息文件地址

} App_information_Str;

extern App_information_Str System_infor;

/*跳转到APP应用*/
void User_App_Jump_Init(void);

/*启动跳转到APP*/
void User_App_Jump_Start(void);

/*备份MCU数据至外部Flash*/
uint8_t User_App_MCU_Flash_Copy(void);

/*外部Flash备份数据写入MCU Flash*/
uint8_t User_App_Flash_Copy(void); 

/*擦除APP占用扇区*/
uint8_t User_App_MCU_Flash_Erase(uint32_t APP_File_Size);

/* 从串口接收APP数据 并写入内部FALSH中*/
void User_App_MCU_Flash_Updata(uint8_t *data , uint16_t len);

/* 从串口接收APP数据 并写入内部FALSH完成*/
void User_APP_MCU_Flash_Finished(void);

/*CRC校验写入MUC Flash数据*/
uint8_t User_App_MCU_Flash_CRC(uint32_t APP_File_Size);



/*初始化Boot*/
void User_Boot_Init(void);


/*Flash写入是否出错*/
extern uint8_t Flash_Error;

/*Flash写入是否完成*/
extern uint8_t Flash_Finished;

#endif

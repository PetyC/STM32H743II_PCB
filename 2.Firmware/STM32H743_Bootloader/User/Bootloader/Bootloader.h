/*
 * @Description: Bootloader跳转到APP程序
 * @Autor: Pi
 * @Date: 2022-07-01 16:53:43
 * @LastEditTime: 2022-07-01 22:13:09
 */
#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "main.h"

/* APP起始地址 */
#define APP_ADDR  0x8100000  

/*系统状态存储*/
extern uint32_t g_JumpInit;

/*系统当前位于状态*/
typedef enum 
{
  Update_APP,           //需要升级APP
  Update_Error_APP,     //升级APP出错
  Jump_APP,             //跳入APP
}SYS_State;



void Jump_To_App(void);

#endif

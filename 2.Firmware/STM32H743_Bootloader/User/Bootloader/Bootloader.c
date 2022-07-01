/*
 * @Description: Bootloader跳转到APP程序
 * @Autor: Pi
 * @Date: 2022-07-01 16:53:36
 * @LastEditTime: 2022-07-01 22:17:16
 */
#include "Bootloader.h"

uint32_t g_JumpInit __attribute__((at(0x20000000), zero_init));

/**
 * @brief 跳转到APP应用
 * @return {*}
 */

void Jump_To_App(void)
{
  /* 声明一个函数指针 */
  void (*AppJump)(void);

  /* 跳转到应用程序，首地址是MSP，地址+4是复位中断服务程序地址 */
  AppJump = (void (*)(void))(*((uint32_t *)(APP_ADDR + 4)));

  /* 设置主堆栈指针 */
  __set_MSP(*(uint32_t *)APP_ADDR);

  /* 跳转到系统BootLoader */
  AppJump();

  /* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
  while (1)
  {
  }
}





void Jump_APP_(void)
{
  if(g_JumpInit == Jump_APP)
  {
    /*跳入APP*/
    Jump_To_App();
  }
}



void Demo()
{
  switch (g_JumpInit)
  {
  case Jump_APP:
    /*跳入APP*/
    Jump_To_App();
    break;
  case Update_APP:
    

    break;

  case Update_Error_APP:

    break;

  default:
    while (1);
    
    break;
  }
}
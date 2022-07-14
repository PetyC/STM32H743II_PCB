/*
 * @Descripttion:
 * @version: HAL STM32Cubemx
 * @Author: Pi
 * @Date: 2021-07-05 15:34:21
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-07-05 15:31:22
 */
#include "app_execute.h"
#include "app_lcd.h"
#include "app_uart.h"
 

Timer SYS_LED_Timer; //系统LED灯定时器

/**
 * @msg: 应用初始化
 * @param {*}
 * @return {*}
 */
void app_execute_init(void)
{
  /*LCD初始化*/
  ST7735S_Init();
  /*UI初始化*/
  UI_Init_Display();
}

/**
 * @msg: 相关软定时器初始化
 * @param {*}
 * @return {*}
 */
void app_execute_time_init(void)
{
  timer_init(&SYS_LED_Timer, SYS_LED_Blink, 0, 500);
  timer_start(&SYS_LED_Timer);
}

/**
 * @msg: 需要一直执行的函数
 * @param {*}
 * @return {*}
 */
void app_execute_loop(void)
{

  //User_UART_Process();
 
}

/**
 * @description: 系统指示灯函数
 * @param {*}
 * @return {*}
 */
void SYS_LED_Blink(void)
{
  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin); //红色LED
}

/*
 * @Descripttion: 
 * @version: HAL STM32Cubemx
 * @Author: Pi
 * @Date: 2021-06-30 12:20:08
 * @LastEditors: Pi
 * @LastEditTime: 2021-07-05 15:42:51
 */

#ifndef APP_H
#define APP_H

#include "stm32H7xx_hal.h"
#include "main.h"

#include "app_core_timer.h"
#include "app_execute.h"


#define CUSTOM_TICK     0       //是否自定义APP TICK

#if CUSTOM_TICK         
#define APP_TICK 1      //软定时器基数
#endif


void app_init(void);
void app_loop(void);
void app_time_core_Tick(void);





#endif

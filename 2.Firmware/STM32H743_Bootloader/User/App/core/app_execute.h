/*
 * @Descripttion: 
 * @version: HAL STM32Cubemx
 * @Author: Pi
 * @Date: 2021-07-05 15:34:47
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-11-10 13:48:23
 */
#ifndef APP_EXECUTE_H
#define APP_EXECUTE_H

#include "app.h"
#include "app_core_timer.h"



void app_execute_loop(void);
void app_execute_time_init(void);
void app_execute_init(void);

void SYS_LED_Blink(void);
void SYS_Heartbeat_APP(void);
void SYS_FSM_Monitor_APP(void);
#endif

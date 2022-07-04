/*
 * @Descripttion: 
 * @version: HAL STM32Cubemx
 * @Author: Pi
 * @Date: 2021-06-30 12:19:48
 * @LastEditors: Pi
 * @LastEditTime: 2021-07-05 15:42:32
 */

#include "app.h"




void app_init(void)
{
    app_execute_init();     
    app_execute_time_init();        //软定时器初始化
}


/**
 * @msg: 循环 一直循环
 * @param {*}
 * @return {*}
 */
void app_loop(void)
{

    #if CUSTOM_TICK
    app_time_core_Tick();
    #endif
    
    app_execute_loop();
    timer_loop();
}




/**
 * @msg: 软定时器内核
 * @param {*}
 * @return {*}
 */
void app_time_core_Tick(void)
{
    #if CUSTOM_TICK 

    static uint32_t time_ms = HAL_GetTick();        //获取系统TICK

    static uint32_t last_ms;
    
    if(time_ms - last_ms <= APP_TICK)
    {
        return ;
    }
    

    last_ms = time_ms;

    #endif

    timer_ticks();
    
}


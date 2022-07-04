/*
 * @Descripttion: 链表实现软定时器功能
 * @version: HAL STM32Cubemx
 * @Author: Pi
 * @Date: 2021-06-29 13:42:46
 * @LastEditors: Pi
 * @LastEditTime: 2021-07-09 11:54:58
 */

 
#ifndef APP_CORE_TIMER_H_
#define APP_CORE_TIMER_H_

#include "stdint.h"
#include "stddef.h"

typedef struct Timer {
    uint32_t timeout;
    uint32_t repeat;
    void (*timeout_cb)(void);
    struct Timer* next;
}Timer;



void timer_init(struct Timer* handle, void(*timeout_cb)(), uint32_t timeout, uint32_t repeat);
int  timer_start(struct Timer* handle);
void timer_stop(struct Timer* handle);
void timer_ticks(void);
void timer_loop(void);

// void timer_again(struct Timer* handle);
// void timer_set_repeat(struct Timer* handle, uint32_t repeat);



#endif

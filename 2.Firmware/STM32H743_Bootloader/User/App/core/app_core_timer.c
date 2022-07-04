/*
 * @Descripttion: 链表实现软定时器功能
 * @version: HAL STM32Cubemx
 * @Author: Pi
 * @Date: 2021-06-29 13:42:46
 * @LastEditors: Pi
 * @LastEditTime: 2021-07-09 11:54:43
 */

#include "app_core_timer.h"

//列表头指针
static struct Timer* head_handle = NULL;

//Timer Tick
static uint32_t _timer_ticks = 0;

/**
  * @brief  初始化定时器结构句柄
  * @param  handle: 计时器句柄strcut
  * @param  timeout_cb: 定时器回调处理函数     
  * @param  timeout:  延迟启动时间      
  * @param  repeat: 循环定时触发时间
  * @retval None
  */
void timer_init(struct Timer* handle, void (*timeout_cb)(), uint32_t timeout, uint32_t repeat)
{
    // memset(handle, sizeof(struct Timer), 0);
    handle->timeout_cb = timeout_cb;
    handle->timeout    = _timer_ticks + timeout;
    handle->repeat     = repeat;
}

/**
  * @brief  开始定时器，并加入链表中
  * @param  btn: handle strcut.
  * @retval 0: succeed. -1: 已存在
  */
int timer_start(struct Timer* handle)
{
    struct Timer* target = head_handle;
    while(target)
    {
        if(target == handle)
            return -1;  //已存在.
        target = target->next;
    }
    handle->next = head_handle;
    head_handle  = handle;
    return 0;
}

/**
  * @brief  关闭定时器并从链表中移除.
  * @param  handle
  * @retval None
  */
void timer_stop(struct Timer* handle)
{
    struct Timer** curr;
    for(curr = &head_handle; *curr;)
    {
        struct Timer* entry = *curr;
        if(entry == handle)
        {
            *curr = entry->next;
            //			free(entry);
        }
        else
            curr = &entry->next;
    }
}

/**
  * @brief  main loop.
  * @param  None.
  * @retval None
  */
void timer_loop()
{
    struct Timer* target;
    for(target = head_handle; target; target = target->next)
    {
        if(_timer_ticks >= target->timeout)
        {
            if(target->repeat == 0)
            {
                timer_stop(target);
            }
            else
            {
                target->timeout = _timer_ticks + target->repeat;
            }
            target->timeout_cb();
        }
    }
}

/**
  * @brief  Tick计时，默认时基1ms
  * @param  None.
  * @retval None.
  */
void timer_ticks()
{
    _timer_ticks++;
}


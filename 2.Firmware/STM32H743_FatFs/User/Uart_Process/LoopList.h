/*
 * @Descripttion: 
 * @version: HAL STM32F1_V1.83
 * @Author: 皮
 * @Date: 2021-06-11 17:21:24
 * @LastEditors: 皮
 * @LastEditTime: 2021-06-25 13:42:24
 */
#ifndef LOOPLIST_H_
#define LOOPLIST_H_

#include <stm32H7xx.h> 
	

#define min(a, b) (a)<(b)?(a):(b)                   ///< 获取最小值

/** 环形缓冲区数据结构 */
typedef struct {
    uint32_t  rbCapacity;//空间大小
    char  *rbHead;			//头
    char  *rbTail;			//尾
    char  *rbBuff;			//数组的首地址
}rb_t;


void rbCreate(rb_t *rb,void *Buff,uint32_t BuffLen);//创建或者说初始化环形缓冲区
void rbDelete(rb_t* rb);
int32_t rbCapacity(rb_t *rb);       //得到环形大小
int32_t rbCanRead(rb_t *rb);        //能读出数据的个数
int32_t rbCanWrite(rb_t *rb);       //还剩余的空间
int32_t rbRead(rb_t *rb, void *data, uint32_t count);//读取数据
int32_t rbWrite(rb_t *rb, const void *data, uint32_t count);
int32_t PutData(rb_t *rb ,void *buf, uint32_t len);


#endif

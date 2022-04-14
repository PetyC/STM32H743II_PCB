/*
 * @Description: 环形队列操作函数
 * @Autor: Pi
 * @Date: 2022-04-14 19:38:30
 * @LastEditTime: 2022-04-14 19:45:27
 */
#ifndef BUFFMANAGE_H_
#define BUFFMANAGE_H_

#include <stm32H7xx.h>
#include "LoopList.h"
#include <stdio.h>


typedef struct{
	signed int  Count;
	signed int  Cnt;
	unsigned char ReadFlage;
	unsigned char SendFlage;
	
	signed int  ReadLen;
	signed int  SendLen;	
	//用户可自由使用以上变量
		
	int32_t value; //内部使用,用户不可使用
	signed int  Len;	//内部使用,用户不可使用
	rb_t Buff;        //管理:缓存数据数组
	rb_t ManageBuff;  //管理:每次缓存数据的数组
}Buff_Manage_Str;



void BufferManageCreate(Buff_Manage_Str *bms,void *buff,uint32_t BuffLen,void *ManageBuff,uint32_t ManageBuffLen);
void BufferManageWrite(Buff_Manage_Str *bms,void *buff,uint32_t BuffLen,int *DataLen);
void BufferManageRead(Buff_Manage_Str *bms,void *buff,int *DataLen);

#endif


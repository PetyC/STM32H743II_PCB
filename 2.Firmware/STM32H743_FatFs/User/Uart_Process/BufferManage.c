/*
 * @Description: 环形队列操作函数
 * @Autor: Pi
 * @Date: 2022-04-14 19:38:30
 * @LastEditTime: 2022-04-15 15:56:58
 */
#include "BufferManage.h"

/**
 * @brief   创建数据缓存管理
 * @param   bms			     缓存管理结构体变量
 * @param   buff          用于缓存数据的数组
 * @param   BuffLen       用于缓存数据的数组的长度
 * @param   ManageBuff    用于记录每次缓存多少字节的数组
 * @param   ManageBuffLen 用于记录每次缓存多少字节的数组长度
 * @retval  None
 **/
void BufferManageCreate(Buff_Manage_Str *bms, void *buff, uint32_t BuffLen, void *ManageBuff, uint32_t ManageBuffLen)
{
	__disable_irq();
	rbCreate(&(bms->Buff), buff, BuffLen);
	rbCreate(&(bms->ManageBuff), ManageBuff, ManageBuffLen);

//	bms->Count = 0;
//	bms->Cnt = 0;
//	bms->ReadFlage = 0;
//	bms->SendFlage = 0;
	bms->ReadLen = 0;
	bms->SendLen = 0;
	bms->value = 0;
	__enable_irq();
}

/**
 * @brief   写入缓存数据
 * @param   bms			缓存管理结构体变量
 * @param   buff     写入的数据
 * @param   BuffLen  写入的数据个数
 * @param   DataLen  返回: 0 Success;1:管理缓存满;2:数据缓存满
 * @retval  None
 **/
void BufferManageWrite(Buff_Manage_Str *bms, void *buff, uint32_t BuffLen, int *DataLen)
{
	__disable_irq();
	if (rbCanWrite(&(bms->Buff)) > BuffLen) //可以写入数据
	{
		if (rbCanWrite(&(bms->ManageBuff)) > 4) //可以记录数据个数
		{
			PutData(&(bms->Buff), buff, BuffLen);
			PutData(&(bms->ManageBuff), &BuffLen, 4);
			*DataLen = 0;
		}
		else
		{
			*DataLen = -1;
		}
	}
	else
	{
		*DataLen = -2;
	}
	__enable_irq();
}

/**
 * @brief   从缓存中读取数据
 * @param   bms			缓存管理结构体变量
 * @param   buff     返回的数据地址
 * @param   DataLen  读取的数据个数
 * @retval  取出的数据个数
 **/
void BufferManageRead(Buff_Manage_Str *bms, void *buff, int *DataLen)
{
	__disable_irq();
	*DataLen = 0;
	if (rbCanRead(&(bms->ManageBuff)) >= 4)
	{
		rbRead(&(bms->ManageBuff), &(bms->Len), 4); //读出来存入的数据个数
		if (bms->Len > 0)
		{
			*DataLen = rbRead(&(bms->Buff), buff, bms->Len);
		}
	}
	__enable_irq();
}

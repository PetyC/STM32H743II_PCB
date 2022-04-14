/*
V1.0.2:
1.屏蔽printf打印
2.设置不同的返回值,以确定具体错误
*/
#include "LoopList.h"

#include <string.h>
#include <stdio.h>

//创建或者说初始化环形缓冲区
void rbCreate(rb_t* rb,void *Buff,uint32_t BuffLen)
{
	if(NULL == rb)
	{
//			printf("ERROR: input rb is NULL\n");
			return;
	}
	rb->rbCapacity = BuffLen;
	rb->rbBuff = Buff;
	rb->rbHead = rb->rbBuff;//头指向数组首地址
	rb->rbTail = rb->rbBuff;//尾指向数组首地址
}

//删除一个环形缓冲区
void rbDelete(rb_t* rb)
{
    if(NULL == rb)
    {
//        printf("ERROR: input rb is NULL\n");
        return;
    }

    rb->rbBuff = NULL;//地址赋值为空
    rb->rbHead = NULL;//头地址为空
    rb->rbTail = NULL;//尾地址尾空
    rb->rbCapacity = 0;//长度为空
}

//获取链表的长度
int32_t rbCapacity(rb_t *rb)
{
    if(NULL == rb)
    {
//        printf("ERROR: input rb is NULL\n");
        return -51;
    }

    return rb->rbCapacity;
}

//返回能读的空间
int32_t rbCanRead(rb_t *rb)
{
    if(NULL == rb)
    {
//        printf("ERROR: input rb is NULL\n");
        return -31;
    }

    if (rb->rbHead == rb->rbTail)//头与尾相遇
    {
        return 0;
    }

    if (rb->rbHead < rb->rbTail)//尾大于头
    {
        return rb->rbTail - rb->rbHead;
    }

    return rbCapacity(rb) - (rb->rbHead - rb->rbTail);//头大于尾
}

//返回能写入的空间
int32_t rbCanWrite(rb_t *rb)
{
    if(NULL == rb)
    {
//        printf("ERROR: input rb is NULL\n");
        return -41;
    }

    return rbCapacity(rb) - rbCanRead(rb);//总的减去已经写入的空间
}

/*   
  rb--要读的环形链表
  data--读出的数据
  count--读的个数
*/
int32_t rbRead(rb_t *rb, void *data, uint32_t count)
{
    int copySz = 0;

    if(NULL == rb)//        printf("ERROR: input rb is NULL\n");
    {
        return -21;
    }

    if(NULL == data)//        printf("ERROR: input data is NULL\n");
    {
        return -22;
    }
		
    if (rb->rbHead < rb->rbTail)//尾大于头
    {
        copySz = min(count, rbCanRead(rb));//查看能读的个数
        memcpy(data, rb->rbHead, copySz);//读出数据到data
        rb->rbHead += copySz;//头指针加上读取的个数
        return copySz;//返回读取的个数
    }
    else //头大于等于了尾
    {
        if (count < rbCapacity(rb)-(rb->rbHead - rb->rbBuff))//读的个数小于头上面的数据量
        {
            copySz = count;//读出的个数
            memcpy(data, rb->rbHead, copySz);//
            rb->rbHead += copySz;
            return copySz;
        }
        else//读的个数大于头上面的数据量
        {
            copySz = rbCapacity(rb) - (rb->rbHead - rb->rbBuff);//先读出来头上面的数据
            memcpy(data, rb->rbHead, copySz);
            rb->rbHead = rb->rbBuff;//头指针指向数组的首地址
					                                           //还要读的个数
            copySz += rbRead(rb, (char*)data+copySz, count-copySz);//接着读剩余要读的个数
            return copySz;
        }
    }
}

int32_t rbWrite(rb_t *rb, const void 
	*data, uint32_t count)
{
    int tailAvailSz = 0;

    if(NULL == rb)
    {
//        printf("ERROR: rb is empty \n");
        return -11;
    }

    if(NULL == data)
    {
//        printf("ERROR: data is empty \n");
        return -12;
    }

    if (count >= rbCanWrite(rb))//如果剩余的空间不够
    {
//        printf("ERROR: no memory \n");
        return -13;
    }

    if (rb->rbHead <= rb->rbTail)//头小于等于尾
    {
        tailAvailSz = rbCapacity(rb) - (rb->rbTail - rb->rbBuff);//查看尾上面剩余的空间
        if (count <= tailAvailSz)//个数小于等于尾上面剩余的空间
        {
            memcpy(rb->rbTail, data, count);//拷贝数据到环形数组
            rb->rbTail += count;//尾指针加上数据个数
            if (rb->rbTail == rb->rbBuff+rbCapacity(rb))//正好写到最后
            {
                rb->rbTail = rb->rbBuff;//尾指向数组的首地址
            }
            return count;//返回写入的数据个数
        }
        else
        {
            memcpy(rb->rbTail, data, tailAvailSz);//填入尾上面剩余的空间
            rb->rbTail = rb->rbBuff;//尾指针指向数组首地址
                   //剩余空间                   剩余数据的首地址       剩余数据的个数
            return tailAvailSz + rbWrite(rb, (char*)data+tailAvailSz, count-tailAvailSz);//接着写剩余的数据
        }
    }
    else //头大于尾
    {
      memcpy(rb->rbTail, data, count);
      rb->rbTail += count;
      return count;
    }
}
/**@} */

/**
* @brief   往环形队列里面写入数据
* @param   rb      环形队列管理变量 
* @param   buf     数据地址
* @param   len     数据长度
* @retval  负数:错误   正数:写入的数据长度
* @warning
* @example 
**/
int32_t PutData(rb_t *rb ,void *buf, uint32_t len)
{
    int32_t count = 0;

    if(NULL == buf)
    {
//        printf("ERROR: gizPutData buf is empty \n");
        return -1;
    }
    
    count = rbWrite(rb, buf, len);
    if(count != len)
    {
        //printf("ERROR: Failed to rbWrite \n");
        return -2;
    }
    return count;
}



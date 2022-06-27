/*
 * @Description: LCD显示任务
 * @Autor: Pi
 * @Date: 2022-06-27 15:14:05
 * @LastEditTime: 2022-06-27 20:04:37
 */
#include "LCD_Task.h"

#include "stdio.h"

void Demo(void);


/**
 * @brief LCD任务
 * @param {void*} argument
 * @return {*}
 */
void LCD_Task(void const *argument)
{

  //设置显示方向
  setOrientation(R0);

  setColor(0, 0, 255);
  fillScreen();

  setColor(31, 63, 31);
  setbgColor(0, 0, 255);
  setFont(ter_u12b);
  drawText(0, 0, "name             Use");
  
  flushBuffer();

  //char pcWriteBuffer[512];


  //User_UART_Write(&huart1, (uint8_t *)pcWriteBuffer, strlen(pcWriteBuffer));

  /* Infinite loop */
  for (;;)
  {
    Demo();
    osDelay(100);
  }
}




void Demo(void)
{

  TaskStatus_t *pxTaskStatusArray;
  volatile UBaseType_t uxArraySize, x;
  unsigned long ulTotalRunTime, ulStatsAsPercentage;

  /*获取任务数量*/
  uxArraySize = uxTaskGetNumberOfTasks();

  /*为每个任务分配一个TaskStatus_t结构*/
  pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

  if (pxTaskStatusArray == NULL)
  {
    return;
  }

  /*生成有关每个任务的原始状态信息*/
  uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

  /* 用于百分比计算 */
  ulTotalRunTime /= 100UL;

  /* 避免除以零错误 */
  if (ulTotalRunTime > 0)
  {
    setColor(31, 63, 31);
    setbgColor(0, 0, 255);
    setFont(ter_u12b);

    /* 对于 pxTaskStatusArray 数组中的每个填充位置,将原始数据格式化为人类可读的 ASCII 数据。 */
    for (x = 0; x < uxArraySize; x++)
    {
      /* 任务使用了总运行时间的百分之几？这将始终向下舍入到最接近的整数。ulTotalRunTimeDiv100 已经除以 100。 */
      ulStatsAsPercentage = pxTaskStatusArray[x].ulRunTimeCounter / ulTotalRunTime;
      
      if (ulStatsAsPercentage > 0UL)
      {
        /*
        sprintf(pcWriteBuffer, "%stt%lutt%lu%%rn",
                pxTaskStatusArray[x].pcTaskName,
                pxTaskStatusArray[x].ulRunTimeCounter,
                ulStatsAsPercentage);
        */
        char Buffer[30];
        sprintf(Buffer , "%c*10" , pxTaskStatusArray[x].pcTaskName);
        drawText(0, 12 + x*12, Buffer);         //显示任务名称
        
      
        sprintf(Buffer , "%d%% " , ulStatsAsPercentage);
        drawText(102, 12 +  x*12 , Buffer);                         //显示CPU使用情况

        flushBuffer();

      }
      else
      {
        drawText(0, 12 +  x*12, pxTaskStatusArray[x].pcTaskName);        //显示任务名称
        drawText(102, 12 + x*12 , "<1% ");        //显示任务名称
        flushBuffer();
      }

      //pcWriteBuffer += strlen((char *)pcWriteBuffer);
    }
  }



  
  /*释放内存*/
  vPortFree(pxTaskStatusArray);
}

/*
u32 TotalRunTime;
UBaseType_t ArraySize,x;
TaskStatus_t *StatusArray;
//第一步:函数uxTaskGetSystemState()的使用

ArraySize=uxTaskGetSystemState((TaskStatus_t* )StatusArray,(UBaseType_t)ArraySize,(uint32_t*)&TotalRunTime);

printf("TaskName\t\tPriority\t\tTaskNumber\t\t\r\n");

for(x=0;x<ArraySize;x++)
{
//通过串口打印出获取到的系统的有关信息 比如任务名称,
//任务优先级和任务编号
printf("%s\t\t%d\t\t\t%d\t\t\t\r\n",
StatusArray[x].pcTaskName,
(int)StatusArray[x].uxCurrentPriority,
(int)StatusArray[x].xTaskNumber);
}


vPortFree(StatusArray);  //释放内存

*/

#include "stm_flash.h"

/*内部调用的函数*/
static uint32_t STMFLASH_ReadWord(uint32_t faddr);		  	//读出字  




/**
 * @brief 获取某个地址所在的flash扇区
 * @param {uint32_t} addr	flash地址
 * @return {uint16_t}	扇区值
 */
uint16_t STMFLASH_GetFlashSector(uint32_t addr)
{
	if (addr < ADDR_FLASH_SECTOR_1_BANK1)
		return FLASH_SECTOR_0;
	else if (addr < ADDR_FLASH_SECTOR_2_BANK1)
		return FLASH_SECTOR_1;
	else if (addr < ADDR_FLASH_SECTOR_3_BANK1)
		return FLASH_SECTOR_2;
	else if (addr < ADDR_FLASH_SECTOR_4_BANK1)
		return FLASH_SECTOR_3;
	else if (addr < ADDR_FLASH_SECTOR_5_BANK1)
		return FLASH_SECTOR_4;
	else if (addr < ADDR_FLASH_SECTOR_6_BANK1)
		return FLASH_SECTOR_5;
	else if (addr < ADDR_FLASH_SECTOR_7_BANK1)
		return FLASH_SECTOR_6;
	else if (addr < ADDR_FLASH_SECTOR_0_BANK2)
		return FLASH_SECTOR_7;
	else if (addr < ADDR_FLASH_SECTOR_1_BANK2)
		return FLASH_SECTOR_0;
	else if (addr < ADDR_FLASH_SECTOR_2_BANK2)
		return FLASH_SECTOR_1;
	else if (addr < ADDR_FLASH_SECTOR_3_BANK2)
		return FLASH_SECTOR_2;
	else if (addr < ADDR_FLASH_SECTOR_4_BANK2)
		return FLASH_SECTOR_3;
	else if (addr < ADDR_FLASH_SECTOR_5_BANK2)
		return FLASH_SECTOR_4;
	else if (addr < ADDR_FLASH_SECTOR_6_BANK2)
		return FLASH_SECTOR_5;
	else if (addr < ADDR_FLASH_SECTOR_7_BANK2)
		return FLASH_SECTOR_6;
	else
		return FLASH_SECTOR_7;

}

//从指定地址开始写入指定长度的数据
//特别注意:因为STM32H7的扇区实在太大,没办法本地保存扇区数据,所以本函数
//         写地址如果非0XFF,那么会先擦除整个扇区且不保存扇区数据.所以
//         写非0XFF的地址,将导致整个扇区数据丢失.建议写之前确保扇区里
//         没有重要数据,最好是整个扇区先擦除了,然后慢慢往后写.
//该函数对OTP区域也有效!可以用来写OTP区!
// OTP区域地址范围:0X1FF0F000~0X1FF0F41F
// WriteAddr:起始地址(此地址必须为4的倍数!!)
// pBuffer:数据指针
// NumToWrite:字(32位)数(就是要写入的32位数据的个数.)
void STMFLASH_Write(uint32_t WriteAddr, uint32_t *pBuffer, uint32_t NumToWrite)
{
	FLASH_EraseInitTypeDef FlashEraseInit;
	HAL_StatusTypeDef FlashStatus = HAL_OK;
	uint32_t SectorError = 0;
	uint32_t addrx = 0;
	uint32_t endaddr = 0;
	if (WriteAddr < STM32_FLASH_BASE || WriteAddr % 4)
		return; //非法地址

	HAL_FLASH_Unlock();										//解锁
	addrx = WriteAddr;										//写入的起始地址
	endaddr = WriteAddr + NumToWrite * 4; //写入的结束地址

	if (addrx < 0X1FF00000)								//bank1
	{
		while (addrx < endaddr) //扫清一切障碍.(对非FFFFFFFF的地方,先擦除)
		{
			if (STMFLASH_ReadWord(addrx) != 0XFFFFFFFF) //有非0XFFFFFFFF的地方,要擦除这个扇区
			{
				FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;			//擦除类型，扇区擦除
				FlashEraseInit.Sector = STMFLASH_GetFlashSector(addrx); //要擦除的扇区
				FlashEraseInit.Banks = FLASH_BANK_1;										//操作BANK1
				FlashEraseInit.NbSectors = 1;														//一次只擦除一个扇区
				FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;		//电压范围，VCC=2.7~3.6V之间!!
				if (HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError) != HAL_OK)
				{
					break; //发生错误了
				}
				//SCB_CleanInvalidateDCache(); //清除无效的D-Cache
			}
			else
				addrx += 4;
			FLASH_WaitForLastOperation(FLASH_WAITETIME, FLASH_BANK_1); //等待上次操作完成
		}
	}
	FlashStatus = FLASH_WaitForLastOperation(FLASH_WAITETIME, FLASH_BANK_1); //等待上次操作完成
	if (FlashStatus == HAL_OK)
	{
		while (WriteAddr < endaddr) //写数据
		{
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, WriteAddr, (uint64_t)pBuffer) != HAL_OK) //写入数据
			{
				break; //写入异常
			}
			WriteAddr += 32;
			pBuffer += 8;
		}
	}
	HAL_FLASH_Lock(); //上锁
}




/**
 * @brief 从指定地址开始读出指定长度的数据
 * @param {uint32_t} ReadAddr	起始地址
 * @param {uint32_t} *pBuffer	读出数据缓存
 * @param {uint32_t} NumToRead	pBuffer长度(字数)
 * @return {*}
 */
void STMFLASH_Read(uint32_t ReadAddr, uint32_t *pBuffer, uint32_t NumToRead)
{
	uint32_t i;
	for (i = 0; i < NumToRead; i++)
	{
		pBuffer[i] = STMFLASH_ReadWord(ReadAddr); //读取4个字节.
		ReadAddr += 4;														//偏移4个字节.
	}
}



/**
 * @brief 读取指定地址的字(32位数据)
 * @param {uint32_t} faddr	读地址
 * @return {uint32_t}	对应数据
 */
static uint32_t STMFLASH_ReadWord(uint32_t faddr)
{
	return *(__IO uint32_t *)faddr;
}



/**
 * @brief 擦除地址所在扇区
 * @param {uint32_t} Addr	需要擦除数据所在地址
 * @param {uint32_t} Sectors_Number	擦除扇区数(1扇区128KB)
 * @return {*}
 */
uint8_t User_Flash_Erase(uint32_t Addr, uint32_t Sectors_Number)
{

	if (Addr < STM32_FLASH_BASE || Addr % 4)
		return 1; //非法地址

	FLASH_EraseInitTypeDef FlashEraseInit;
	HAL_StatusTypeDef FlashStatus = HAL_OK;
	uint32_t SectorError = 0;

	uint16_t BANK_Number = 1;

	if(Addr >= ADDR_FLASH_SECTOR_0_BANK2 )					//判断BANK
	{
		BANK_Number = FLASH_BANK_2;										//操作BANK2
	}
	else 
	{
		BANK_Number = FLASH_BANK_1;										//操作BANK1
	}
	
	
	__set_PRIMASK(1);    //关闭STM32总中断


	HAL_FLASH_Unlock();										//解锁

	FlashEraseInit.Banks = BANK_Number;											//操作BANK2
	FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;			//擦除类型，扇区擦除
	FlashEraseInit.Sector = STMFLASH_GetFlashSector(Addr); //要擦除的扇区
	FlashEraseInit.NbSectors = Sectors_Number;							//擦除页数
	FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;		//电压范围，VCC=2.7~3.6V之间!!

	FlashStatus = HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError); 		//进行擦除
	FLASH_WaitForLastOperation(FLASH_WAITETIME, BANK_Number); //等待上次操作完成
	HAL_FLASH_Lock(); //上锁

	__set_PRIMASK(0);    //开启STM32总中断

	if(SectorError == 0xFFFFFFFF && FlashStatus == HAL_OK)
	{
		return 0;
	}
	else
	{
		return 1;
	}

}



/**
 * @brief 擦除后写入数据
 * @param {uint32_t} WriteAddr	写入地址
 * @param {uint32_t} *pBuffer		写入数据
 * @param {uint32_t} pBuffer_Len	数据长度
 * @return {uint8_t} 0:成功			1:失败
 */
uint8_t User_Flash_Write(uint32_t WriteAddr, uint32_t *pBuffer, uint32_t pBuffer_Len)
{
	uint32_t endaddr = 0;
	if (WriteAddr < STM32_FLASH_BASE || WriteAddr % 4)
		return 1; //非法地址

	endaddr = WriteAddr + pBuffer_Len * 4; //写入的结束地址

	uint16_t Sectors_End_Number = STMFLASH_GetFlashSector(endaddr);
	uint16_t Sectors_Number = Sectors_End_Number - STMFLASH_GetFlashSector(WriteAddr) + 1;
	
	if(User_Flash_Erase(WriteAddr , Sectors_Number) == 1)					//先擦除数据
	{
		return 1;			//出错
	}


	__set_PRIMASK(1);    //关闭STM32总中断
	HAL_FLASH_Unlock();										//解锁

	while (WriteAddr < endaddr) //写数据
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, WriteAddr, (uint64_t)pBuffer) != HAL_OK) //以字(32位)写入数据
		{
			return 1; 		//写入异常
		}
		WriteAddr += 32;		//地址递增32位  4*8
		pBuffer += 8;				//因为 uint32_t -> uint64_t 强制转换 
	}
	
	HAL_FLASH_Lock(); //上锁
	
	__set_PRIMASK(0);    //打开STM32总中断
	
	return 0;	
}




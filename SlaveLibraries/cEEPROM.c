/**
  ******************************************************************************
  * @file    IAP_Main/Src/ymodem.c 
  * @author  MCD Application Team
  * @version 1.0.0
  * @date    8-April-2015
  * @brief   This file provides all the software functions related to the ymodem 
  *          protocol.
  *****************************************************************************/

#include "parameter.h"

/*------------------------------------------------------------ 
 Func: EEPROM数据按字节读出 
 Note: 
-------------------------------------------------------------*/  


HAL_StatusTypeDef EEPROM_ReadBytes(uint32_t Address, uint8_t *Buffer, uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_ERROR;
	uint8_t *addr;  
	
  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
  
  if(status == HAL_OK)
  {
    addr=(uint8_t *) Address;
    while(Length--)
		{  
        *Buffer++=*addr++;  
    }     
    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
  }
  return status;
}

uint32_t HAL_FLASH_GetWord(uint32_t Address)
{
	uint32_t data;  

  data = *(__IO uint32_t*)(Address);
  
  return data;
}

uint8_t HAL_FLASH_GetByte(uint32_t Address)
{
	uint8_t data;  

  data = *(__IO uint8_t*)(Address);
  
  return data;
}

//void EEPROM_ReadWords(uint32_t Address, uint32_t *Buffer, uint16_t Length)
//{
//	uint32_t *addr;  

//	addr = (uint32_t*) Address;

//	while(Length--)
//	{  
//			*Buffer++=*addr++;  
//	}   

//}

//uint32_t EEPROM_WriteWords(uint32_t Address, uint32_t *Buffer, uint16_t Length)		//checked hiyangdong 无问题
//{
//  uint16_t i = 0;

//  /* Unlock the Flash to enable the flash control register access *************/
//  HAL_FLASH_Unlock();

//  for (i = 0; (i < Length) && (Address <= (USER_EEPROM_END_ADDRESS-4)); i++)
//  {

//    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, *(uint32_t*)(Buffer+i)) == HAL_OK)      
//    {
//     /* Check the written value */
//      if (*(uint32_t*)Address != *(uint32_t*)(Buffer+i))
//      {
//        /* Flash content doesn't match SRAM content */
//				printf("Flash not match ERROR\r\n");
//        return(FLASHIF_WRITINGCTRL_ERROR);
//      }
//      /* Increment FLASH destination address */
//      Address += 4;
//    }
//    else
//    {
//      /* Error occurred while writing data in Flash memory */
//			printf("Flash ERROR\r\n");
//      return (FLASHIF_WRITING_ERROR);
//    }
//  }

//  /* Lock the Flash to disable the flash control register access (recommended
//     to protect the FLASH memory against possible unwanted operation) *********/
//  HAL_FLASH_Lock();

//  return (FLASHIF_OK);
//}


//uint32_t writeFlashData = 0x55555869;
//uint32_t addr = 0x08013000;

//void printFlashTest(void)
//{
//  uint32_t temp = *(__IO uint32_t*)(addr);

//    printf("addr:0x%x, data:0x%x\r\n", addr, temp);
//}

//uint32_t getFlashData(void)
//{
//  uint32_t temp = *(__IO uint32_t*)(addr);

//    printf("addr:0x%x, data:0x%x\r\n", addr, temp);
//	return temp;
//}

//void writeFlashTest(void)
//{
//    //1、解锁FLASH
//  HAL_FLASH_Unlock();

//    //2、擦除FLASH
//    //初始化FLASH_EraseInitTypeDef
//    FLASH_EraseInitTypeDef f;
//    f.TypeErase = FLASH_TYPEERASE_PAGES;
//    f.PageAddress = addr;
//    f.NbPages = 1;
//    //设置PageError
//    uint32_t PageError = 0;
//    //调用擦除函数
//    HAL_FLASHEx_Erase(&f, &PageError);

//    //3、对FLASH烧写
//    HAL_FLASH_Program(TYPEPROGRAM_WORD, addr, writeFlashData);

//    //4、锁住FLASH
//  HAL_FLASH_Lock();
//}




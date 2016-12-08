/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

#include "main.h"
#include "cLEDs.h"		//代表状态灯
#include "sI2C.h"
#include "sKey.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* Buffer used for transmission */


/* Buffer used for reception */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

#ifdef __GNUC__  
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)  
#else  
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)  
#endif /* __GNUC__ */  
PUTCHAR_PROTOTYPE
{  
	HAL_UART_Transmit(&huart1 , (uint8_t *)&ch, 1, 100);  
	return ch;  
}

#define APPLICATION_ADDRESS				(uint32_t)0x08001000
#define SYSCFG_MemoryRemap_SRAM		((uint8_t)0x03)

#if   (defined ( __CC_ARM ))
  __IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
#elif (defined (__ICCARM__))
#pragma location = 0x20000000
  __no_init __IO uint32_t VectorTable[48];
#elif defined   (  __GNUC__  )
  __IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
#elif defined ( __TASKING__ )
  __IO uint32_t VectorTable[48] __at(0x20000000);
#endif

void NVIC_SRAM(void)
{
	for(uint8_t i = 0; i < 48; i++)
  {
    VectorTable[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));
  }
	
	uint32_t tmpctrl = 0;
  /* Get CFGR1 register value */
  tmpctrl = SYSCFG->CFGR1;

  /* Clear MEM_MODE bits */
  tmpctrl &= (uint32_t) (~SYSCFG_CFGR1_MEM_MODE);

  /* Set the new MEM_MODE bits value */
  tmpctrl |= (uint32_t) SYSCFG_MemoryRemap_SRAM;

  /* Set CFGR1 register with the new memory remap configuration */
  SYSCFG->CFGR1 = tmpctrl;
}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
	NVIC_SRAM();

  if(NVIC_GetPendingIRQ(SysTick_IRQn) != RESET)
  {
    HAL_NVIC_ClearPendingIRQ(SysTick_IRQn);
		HAL_UART_Transmit(&huart1,"SysTick_IRQn\r\n",14,10);
  }
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_TIM17_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim17);

	cLEDs_Init();
	sKey_Init();
	printf("I am sKey\r\n");
	
	HAL_I2C_Slave_Receive_DMA(&hi2c1,i2c_Data.frame,I2C_FRAME_SIZE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while(1)
	{
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
		//此处进入休眠状态
	}
  /* USER CODE END 3 */

}
/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)  //定时中断
{
	HAL_TIM_Base_Start_IT(&htim17);
	if(i2c_Data.cmd == 0x51)
	{
		HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_1);
		i2c_Data.cmd = 0x00;
	}
}

void HAL_SYSTICK_Callback(void)
{
	if(HAL_GetTick()%1000==0)
	{
		cLEDs_Toggle(R);
		
		//如果I2C进入空闲状态，必须重启接收
		if(HAL_I2C_GetState(&hi2c1)==HAL_I2C_STATE_READY)
		{
			HAL_I2C_Slave_Receive_DMA(&hi2c1,i2c_Data.frame,I2C_FRAME_SIZE);
		}
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	i2cdata_parse(&deviceMe,&i2c_Data);
	if(checkCRC(&i2c_Data)==true)
	{
		//广播帧
		if(getI2CHardAddress(&i2c_Data) == I2C_BROADCAST_ADDR)
		{
			deviceMe.command = 0x05;
			i2cdata_update(&deviceMe, &i2c_Data);
			HAL_I2C_Slave_Transmit_DMA(&hi2c1,i2c_Data.frame,I2C_FRAME_SIZE);
		}
		//数据帧
		else if(getI2CHardAddress(&i2c_Data) == SLAVE_HARD_ADDR)
		{
			if(checkI2CSoftAddress(&deviceMe,&i2c_Data)==true)
			{
				printf("OK:0x%x\r\n",i2c_Data.cmd);
				sExecuteComand(&deviceMe, &i2c_Data);
				i2cdata_update(&deviceMe, &i2c_Data);
				HAL_I2C_Slave_Transmit_DMA(&hi2c1,i2c_Data.frame,I2C_FRAME_SIZE);
			}
		}
	}
	else
	{
		printf("======CRC ERROR\r\n");
		HAL_I2C_Slave_Receive_DMA(&hi2c1,i2c_Data.frame,I2C_FRAME_SIZE);
	}
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	printf("HAL_I2C_SlaveTx OK------------\r\n");
	HAL_I2C_Slave_Receive_DMA(&hi2c1,i2c_Data.frame,I2C_FRAME_SIZE);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)  //HAL_I2C_ERROR_BERR
{
	printf("HAL_I2C_ErrorCallback------------\r\n");
	HAL_DMA_Abort(&hdma_i2c1_rx);
	HAL_DMA_Abort(&hdma_i2c1_tx);
	HAL_I2C_Slave_Receive_DMA(&hi2c1,i2c_Data.frame ,I2C_FRAME_SIZE);
}

/* USER CODE END 4 */

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

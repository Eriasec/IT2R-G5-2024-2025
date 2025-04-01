/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @brief   STM32F4xx HAL API Template project 
  *
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
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
#include "main.h"
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C

#include "Board_LED.h"                  // ::Board Support:LED

#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif
#ifdef RTE_CMSIS_RTOS2                  // when RTE component CMSIS RTOS2 is used
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif

#include "stm32f4xx.h"                  // Device header
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
#include <stdio.h>
#include <string.h>


#ifdef RTE_CMSIS_RTOS2_RTX5




/**
  * Override default HAL_GetTick function
  */
uint32_t HAL_GetTick (void) {
  static uint32_t ticks = 0U;
         uint32_t i;

  if (osKernelGetState () == osKernelRunning) {
    return ((uint32_t)osKernelGetTickCount ());
  }

  /* If Kernel is not running wait approximately 1 ms then increment 
     and return auxiliary tick counter value */
  for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  }
  return ++ticks;
}

#endif

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

char X,Y,C,Z;


static void SystemClock_Config(void);
static void Error_Handler(void);



extern ARM_DRIVER_USART Driver_USART2;
extern ARM_DRIVER_I2C Driver_I2C1;

osThreadId tache1_id;

void Init_UART(void);
void UART_Callback(uint32_t event);

osThreadId ID_Tache1 ;
osThreadId ID_Tache2 ;
osThreadId ID_Tache3 ;

void tache1(void const * argument);
void tache2(void const * argument);
void tache3(void const * argument);



void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur)
  {
  unsigned char tab[2];
  tab[0] = registre;
	tab[1] = valeur; 
    
  Driver_I2C1.MasterTransmit (composant&(0x7F), tab, 2, false);
    while (Driver_I2C1.GetStatus().busy == 1);
  }


void write1byte_1(unsigned char composant, unsigned char registre)
  {
  unsigned char tab[2];
  tab[0] = registre;
    
  Driver_I2C1.MasterTransmit (composant&(0x7F), tab, 1, false);
    while (Driver_I2C1.GetStatus().busy == 1);
  }	
	
void read6byte(unsigned char composant, unsigned char * maValeur)
	{
		Driver_I2C1.MasterReceive (composant&(0x7F), maValeur, 6, false);		// false = avec stop
		while (Driver_I2C1.GetStatus().busy == 1);	// attente fin transmission
		
		
	}




void Init_I2C(void)
{



	Driver_I2C1.Initialize(NULL);
	Driver_I2C1.PowerControl(ARM_POWER_FULL);
	Driver_I2C1.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = d�bit
							ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
	//Driver_I2C1.Control(	ARM_I2C_BUS_CLEAR,0 );
}


/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */


osThreadDef (tache1, osPriorityNormal, 1, 0);
osThreadDef (tache2, osPriorityNormal, 1, 0);


int main(void)
{
  HAL_Init();
	
	SystemClock_Config();
  SystemCoreClockUpdate();
	
	Init_UART();
	Init_I2C();
	LED_Initialize ();
	
	osKernelInitialize ();	
	
	ID_Tache1 = osThreadCreate ( osThread ( tache1 ), NULL ) ;
	ID_Tache2 = osThreadCreate ( osThread ( tache2 ), NULL ) ;
	
  osKernelStart();
	osDelay(osWaitForever);
	
  while (1)
  {
	
  }
}

void Init_UART(void){
	Driver_USART2.Initialize(UART_Callback);
	Driver_USART2.PowerControl(ARM_POWER_FULL);
	Driver_USART2.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							115200);
	Driver_USART2.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART2.Control(ARM_USART_CONTROL_RX,1);
}



void UART_Callback(uint32_t event){		
	
		if (event & ARM_USART_EVENT_SEND_COMPLETE)
		{
        osSignalSet(ID_Tache1, 0x0002);
    }
    	
}





void tache1(void const *argument){
		uint8_t rx_buffer[9];
		char START=0xFF;
		// Generation de valeurs fixes
	
		while(1){
			sprintf((char *)rx_buffer, "%d%d%3d%3d\n", C,Z,X,Y);
//			rx_buffer[0]=C;
//			rx_buffer[1]=Z;
//			rx_buffer[2]=X;
//			rx_buffer[3]=Y;

			
			Driver_USART2.Send(&C, 1);
			osSignalWait(0x0002, osWaitForever);
			osDelay(100);
			Driver_USART2.Send(&Z, 1);
			osSignalWait(0x0002, osWaitForever);
			osDelay(100);
			Driver_USART2.Send(&X, 1);
			osSignalWait(0x0002, osWaitForever);
			osDelay(100);
			Driver_USART2.Send(&Y, 1);
			osSignalWait(0x0002, osWaitForever);
			
			
			osDelay(250);
		}
}




void tache2(void const *argument){


		    unsigned char tab6[6]; 
				write1byte(0x52,0xF0,0x55);
				osDelay(50);
				write1byte(0x52,0xFB,0x00);
				osDelay(50);
				

			while(1)
			{

				write1byte_1(0x52,0x00);
				osDelay(100);
				
				read6byte(0x52, tab6);
				osDelay(100);
				
				X = tab6[0];
				Y = tab6[1];
				C = ((~tab6[5])&0x02)>>1;
				Z =  ((~tab6[5])&0x01);
			}
}













/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 25
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

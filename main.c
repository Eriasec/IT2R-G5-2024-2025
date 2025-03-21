#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "adc_F4.h"

#include "main.h"
#include "Board_LED.h"                  // ::Board Support:LED

#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif
#ifdef RTE_CMSIS_RTOS2                  // when RTE component CMSIS RTOS2 is used
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif

#ifdef RTE_CMSIS_RTOS2_RTX5

	
void ADC_Initialize(ADC_HandleTypeDef *ADCHandle,unsigned int channel);
	
	
uint32_t HAL_GetTick (void) {
  static uint32_t ticks = 0U;
         uint32_t i;

  if (osKernelGetState () == osKernelRunning) {
    return ((uint32_t)osKernelGetTickCount ());
  }

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


static void SystemClock_Config(void);
static void Error_Handler(void);
static void ADC_Init(ADC_HandleTypeDef* hadc); 

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
int main(void)
{

  HAL_Init();
	

  SystemClock_Config();
  SystemCoreClockUpdate();

	LED_Initialize();
	

#ifdef RTE_CMSIS_RTOS2	

  osKernelInitialize ();


  osKernelStart();
#endif


  while (1)
  {

  }
}


HAL_StatusTypeDef HAL_ADC_Start	(	ADC_HandleTypeDef * 	hadc	)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  __IO uint32_t counter = 0UL;

  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));

  /* Verify that ADC is not already busy */
  if (hadc->State == HAL_ADC_STATE_READY)
  {
    /* Set ADC state */
    hadc->State = HAL_ADC_STATE_BUSY_REG;

    /* Start conversion if ADC is effectively enabled */
    if (LL_ADC_IsEnabled(hadc->Instance) == 1UL)
    {
      /* Clear regular group conversion flag and overrun flag */
      LL_ADC_ClearFlag_EOC(hadc->Instance);
      LL_ADC_ClearFlag_OVR(hadc->Instance);

      /* Enable conversion of regular group */
      LL_ADC_REG_StartConversion(hadc->Instance);
    }
    else
    {
      /* Enable the ADC peripheral */
      tmp_hal_status = ADC_Enable(hadc);

      /* Check if ADC is effectively enabled */
      if (tmp_hal_status == HAL_OK)
      {
        /* Start conversion of regular group */
        LL_ADC_REG_StartConversion(hadc->Instance);
      }
      else
      {
        /* Update ADC state machine to error */
        hadc->State = HAL_ADC_STATE_ERROR_INTERNAL;
      }
    }
  }
  else
  {
    tmp_hal_status = HAL_BUSY;
  }

  /* Return function status */
  return tmp_hal_status;


}

//HAL_StatusTypeDef HAL_ADC_PollForConversion	(	ADC_HandleTypeDef * 	hadc, uint32_t 	Timeout )
//{
//}


//uint32_t HAL_ADC_GetValue	(	ADC_HandleTypeDef * 	hadc	)	
//{


//}

//HAL_StatusTypeDef HAL_ADC_Stop	(	ADC_HandleTypeDef * 	hadc	)	
//{


//}


void ADC_Initialize(ADC_HandleTypeDef *ADCHandle,unsigned int channel)
{	
	ADC_ChannelConfTypeDef Channel_AN;
	GPIO_InitTypeDef ADCpin; 
	

	__HAL_RCC_ADC1_CLK_ENABLE();
	ADCHandle->Instance = ADC1; // create an instance of ADC1
	ADCHandle->Init.Resolution = ADC_RESOLUTION_12B; // select 12-bit resolution 
	ADCHandle->Init.EOCSelection = ADC_EOC_SINGLE_CONV; //select  single conversion as a end of conversion event
	ADCHandle->Init.DataAlign = ADC_DATAALIGN_RIGHT; // set digital output data right justified
	ADCHandle->Init.ClockPrescaler =ADC_CLOCK_SYNC_PCLK_DIV8; 
	HAL_ADC_Init(ADCHandle); // initialize AD1 with myADC1Handle configuration settings
	
//Channel selection (AIN1 or AIN8)
	switch(channel){
		case 1://AIN1
				__HAL_RCC_GPIOA_CLK_ENABLE(); // enable clock to GPIOA
				ADCpin.Pin = GPIO_PIN_1; // AN1 => PA1 => Select pin 1 from GPIO A
				ADCpin.Mode = GPIO_MODE_ANALOG; // Select Analog Mode
				ADCpin.Pull = GPIO_NOPULL; // Disable internal pull-up or pull-down resistor
				HAL_GPIO_Init(GPIOA, &ADCpin); // initialize PA1 as analog input pin

				Channel_AN.Channel = ADC_CHANNEL_1; // AN1 => select analog channel 1
		break;
		
		case 8://AIN8
				__HAL_RCC_GPIOB_CLK_ENABLE(); // enable clock to GPIOB
				ADCpin.Pin = GPIO_PIN_0; // AN8 => PB0 => Select pin 0 from GPIO B
				ADCpin.Mode = GPIO_MODE_ANALOG; // Select Analog Mode
				ADCpin.Pull = GPIO_NOPULL; // Disable internal pull-up or pull-down resistor
				HAL_GPIO_Init(GPIOB, &ADCpin); // initialize PB8 as analog input pin

				Channel_AN.Channel = ADC_CHANNEL_8; // AN8 => select analog channel 8
	break;
	}

	Channel_AN.Rank = 1; // set rank to 1
	Channel_AN.SamplingTime = ADC_SAMPLETIME_15CYCLES; // set sampling time to 15 clock cycles
	HAL_ADC_ConfigChannel(ADCHandle, &Channel_AN); // select channel_8 for ADC1 module. 
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
  *            PLL_M                          = 8
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

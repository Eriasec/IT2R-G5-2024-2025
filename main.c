






/*************************************  A NOUS ***************************************************************/



#define SLAVE_I2C_ADDR       0x70			// Adresse esclave sur 7 bits
#define capteurE0						 0xE0
#define capteurE2						 0xE2
#define capteurEA						 0xEA
#define capteurEC						 0xEC
#define d_1m						     0x18
#define d_4m						     0x5D
#define d_11m						     0xFF

#define capteur1						 0xE0	
#define capteur2						 0xEC	

/*************************************  A NOUS ***************************************************************/







/************************************* PAS A NOUS ***************************************************************/
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
static void SystemClock_Config(void);
static void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/
/**
	* @brief  Main program
	* @param  None
	* @retval None
	*/
												
												
/************************************* PAS A NOUS ***************************************************************/	




/*************************************  A NOUS ***************************************************************/

uint8_t MSB_1 , MSB_2;
uint8_t LSB_1 , LSB_2;
uint16_t val_1 , val_2;
char X,Y,C,Z;
osThreadId ID_Tache1 ;
osThreadId ID_Tache2 ;
osThreadId ID_Tache3 ;

osMutexId ID_mut_I2c;
osMutexDef (mut_I2c);

void tache1(void const * argument);
void tache2(void const * argument);
void tache3(void const * argument);

osThreadId ID_tache1,ID_tache2,ID_tache3;

extern ARM_DRIVER_I2C Driver_I2C1;

/**********************************FONCTION*****************************************************/
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

void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur)
  {
  unsigned char tab[2];
  tab[0] = registre;
	tab[1] = valeur; 
    
  Driver_I2C1.MasterTransmit (composant>>1&(0x7F), tab, 2, false);
    while (Driver_I2C1.GetStatus().busy == 1);
  }
	
	
unsigned char read1byte(unsigned char composant, unsigned char registre)
  {
    uint8_t  maValeur;
    unsigned char tab[1]; 
    tab[0] = registre;
    Driver_I2C1.MasterTransmit (composant>>1&(0x7F), tab, 1, true);
    while (Driver_I2C1.GetStatus().busy == 1);	// attente fin transmission
    
    Driver_I2C1.MasterReceive (composant>>1&(0x7F), &maValeur, 1, false);		// false = avec stop
		while (Driver_I2C1.GetStatus().busy == 1);	// attente fin transmission
    return maValeur;
  }

void init_capteur(capteur_add,distance)
	{

		write1byte(capteur_add,0x02,distance);	// 0x75
		write1byte(capteur_add,0x01,0x10);
	}
void Init_I2C(void)
  {
  
  

	Driver_I2C1.Initialize(NULL);
	Driver_I2C1.PowerControl(ARM_POWER_FULL);
	Driver_I2C1.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = débit
							ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
	//Driver_I2C1.Control(	ARM_I2C_BUS_CLEAR,0 );
}


/**********************************FONCTION*****************************************************/

void tache1(void const * argument)
{
while(1)
{ 
    osMutexWait(ID_mut_I2c,osWaitForever);//mutexwait
		write1byte(capteur1,0x00,0x51);
		osDelay(70);	
		MSB_1 = read1byte(capteur1,0x02);
		osDelay(2);
		LSB_1 = read1byte(capteur1,0x03);
		val_1 = (((short)MSB_1)<<8) + LSB_1;
		osDelay(50);
    osMutexRelease(ID_mut_I2c);//mutexrelease
}
}
void tache2(void const * argument)
{
  
while(1)
{
    osMutexWait(ID_mut_I2c,osWaitForever);//mutexwait
		write1byte(capteur2,0x00,0x51);
		osDelay(70);	
		MSB_2 = read1byte(capteur2,0x02);
		osDelay(2);
		LSB_2 = read1byte(capteur2,0x03);
		val_2 = (((short)MSB_2)<<8) + LSB_2;
		osDelay(50);
     osMutexRelease(ID_mut_I2c);//mutexrelease
}
}
void tache3(void const * argument)
{		
		unsigned char tab6[6];
		osMutexWait(ID_mut_I2c,osWaitForever); //mutexwait
    
		write1byte(0x52,0xF0,0x55);
		osDelay(50);
		write1byte(0x52,0xFB,0x00);
		osDelay(50);
    osMutexRelease(ID_mut_I2c);//mutexrelease
while(1)
{
  osMutexWait(ID_mut_I2c,osWaitForever);//mutexwait
	write1byte_1(0x52,0x00);
	osDelay(50);
	
	read6byte(0x52, tab6);
	osDelay(100);
	osMutexRelease(ID_mut_I2c);//mutexrelease
	X = tab6[0];
	Y = tab6[1];
	C = ((~tab6[5])&0x02)>>1;
	Z =  ((~tab6[5])&0x01);
}
}

osThreadDef (tache1, osPriorityNormal, 1, 0);
osThreadDef (tache2, osPriorityNormal, 1, 0);
osThreadDef (tache3, osPriorityNormal, 1, 0);









int main(void)
{
	uint8_t tab[10];
	val_1 = 0;
	val_2 = 0;
	HAL_Init();
	
	ID_Tache1 = osThreadCreate ( osThread ( tache1 ), NULL ) ;
	ID_Tache2 = osThreadCreate ( osThread ( tache2 ), NULL ) ;
	ID_Tache3 = osThreadCreate ( osThread ( tache3 ), NULL ) ;

  ID_mut_I2c = osMutexCreate(osMutex(mut_I2c));  
  
	SystemClock_Config();
	SystemCoreClockUpdate();
	
	osKernelInitialize ();
	LED_Initialize ();
	Init_I2C();
	init_capteur(capteur1,d_11m); 
	init_capteur(capteur2,d_11m);
	osKernelStart();
	osDelay(osWaitForever) ;
	
		while (1)
	{ 	
			
	}



}
/*************************************************************************************************************

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

	/************************************* PAS A NOUS ***************************************************************/


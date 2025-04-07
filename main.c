






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

#define ID_lecture_capteur   0x001

/*************************************  A NOUS ***************************************************************/







/************************************* PAS A NOUS ***************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN

#include "Board_LED.h"                  // ::Board Support:LED
#include "Driver_SPI.h"                 // ::CMSIS Driver:SPI

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

osThreadId ID_Tache1 ;
osThreadId ID_Tache2 ;
osThreadId ID_Tache3 ;

osMutexId ID_mut_I2c;
osMutexDef (mut_I2c);

void tache1(void const * argument);
void tache2(void const * argument);
void tache3(void const * argument);
void tache4(void const * argument);

osThreadId ID_tache1,ID_tache2,ID_tache3,ID_tache4;

extern ARM_DRIVER_SPI Driver_SPI1;
extern ARM_DRIVER_I2C Driver_I2C1;
extern ARM_DRIVER_CAN Driver_CAN2;

void Init_CAN(void)
{
Driver_CAN2.Initialize(NULL,NULL);
Driver_CAN2.PowerControl(ARM_POWER_FULL);
Driver_CAN2.SetMode(ARM_CAN_MODE_INITIALIZATION);
Driver_CAN2.SetBitrate
	(
	ARM_CAN_BITRATE_NOMINAL, // débit fixe
	125000, // 125 kbits/s (LS)
	ARM_CAN_BIT_PROP_SEG(5U) | // prop. seg = 5 TQ
	ARM_CAN_BIT_PHASE_SEG1(1U) | // phase seg1 = 1 TQ
	ARM_CAN_BIT_PHASE_SEG2(1U) | // phase seg2 = 1 TQ
	ARM_CAN_BIT_SJW(1U) // Resync. Seg = 1 TQ
	);
	Driver_CAN2.ObjectConfigure(2,ARM_CAN_OBJ_TX);
	Driver_CAN2.ObjectConfigure(0,ARM_CAN_OBJ_RX); // Objet 0 pour réception
	Driver_CAN2.SetMode(ARM_CAN_MODE_NORMAL); // fin initialisation
}
void ledrouge(void)
{	  
	  int i; 
		uint8_t DEBUT[4]={0x00,0x00,0x00,0x00}  ;
		uint8_t LED[4]= {0xE0 | 0x1F, 0x00, 0x00, 0xFF};
		uint8_t FIN[4] = {0xFF, 0xFF, 0xFF, 0xFF};
		
		Driver_SPI1.Send(DEBUT,4);
	
		for (i = 0; i < 60; i++) 
		{
        Driver_SPI1.Send(LED, 4);
    }
    Driver_SPI1.Send(FIN, 4);
}

void ledvert(void)
{	  
	  int i; 
		uint8_t DEBUT[4]={0x00,0x00,0x00,0x00}  ;
		uint8_t LED[4]= {0xE0 | 0x1F, 0x00, 0xFF, 0x00};
		uint8_t FIN[4] = {0xFF, 0xFF, 0xFF, 0xFF};
		
		Driver_SPI1.Send(DEBUT,4);
	
		for (i = 0; i < 60; i++) 
		{
        Driver_SPI1.Send(LED, 4);
    }
    Driver_SPI1.Send(FIN, 4);
}
void ledbleu(void)
{	  
	  int i; 
		uint8_t DEBUT[4]={0x00,0x00,0x00,0x00}  ;
		uint8_t LED[4]= {0xE0 | 0x1F, 0xFF, 0x00, 0x00};
		uint8_t FIN[4] = {0xFF, 0xFF, 0xFF, 0xFF};
		
		Driver_SPI1.Send(DEBUT,4);
	
		for (i = 0; i < 60; i++) 
		{
        Driver_SPI1.Send(LED, 4);
    }
    Driver_SPI1.Send(FIN, 4);
}
void ledrose(void)
{	  
	  int i; 
		uint8_t DEBUT[4]={0x00,0x00,0x00,0x00}  ;
		uint8_t LED[4]= {0xE0 | 0x1F, 0xFF, 0x00, 0xFF};
		uint8_t FIN[4] = {0xFF, 0xFF, 0xFF, 0xFF};
		
		Driver_SPI1.Send(DEBUT,4);
	
		for (i = 0; i < 60; i++) 
		{
Driver_SPI1.Send(LED, 4);
    }
    Driver_SPI1.Send(FIN, 4);
}
void Init_SPI(void)
	{
	Driver_SPI1.Initialize(NULL);		// Fonction callback à definir
	Driver_SPI1.PowerControl(ARM_POWER_FULL);
	Driver_SPI1.Control(ARM_SPI_MODE_MASTER | 
											ARM_SPI_CPOL1_CPHA1 | 			// POL = 1 pour CLK à 1 en idle, PHA = 0 pour lecture sur front montant. de CLK
//											ARM_SPI_MSB_LSB | 
											ARM_SPI_SS_MASTER_UNUSED |
											ARM_SPI_DATA_BITS(8), 4000000);
	Driver_SPI1.Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
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
while(1)
{
	osMutexWait(ID_mut_I2c,osWaitForever);//mutexwait
	if (val_1 <15 || val_2 < 15)
		ledrouge();
	else 
		ledvert();
	osMutexRelease(ID_mut_I2c);//mutexrelease
}
}

void tache4(void const * argument)
{
	char data_buf [50];
	ARM_CAN_MSG_INFO tx_msg_info;
	while(1)
		{
		
		osMutexWait(ID_mut_I2c,osWaitForever);//mutexwait
			
			tx_msg_info.id = ARM_CAN_STANDARD_ID (ID_lecture_capteur);
			tx_msg_info.rtr = 0; // 0 = trame DATA
			data_buf [0] =(char)(val_1<<2); // data à envoyer à placer dans un tableau de char
			Driver_CAN2.MessageSend(2, &tx_msg_info, data_buf, 1); // 1 data à envoyer
		osMutexRelease(ID_mut_I2c);//mutexrelease	
		
		}
}



osThreadDef (tache1, osPriorityNormal, 1, 0);
osThreadDef (tache2, osPriorityNormal, 1, 0);
osThreadDef (tache3, osPriorityNormal, 1, 0);
osThreadDef (tache4, osPriorityNormal, 1, 0);

int main(void)
{
	uint8_t tab[10];
	val_1 = 0;
	val_2 = 0;
	HAL_Init();
	


	ID_mut_I2c = osMutexCreate(osMutex(mut_I2c));  
	
	
	SystemClock_Config();
	SystemCoreClockUpdate();
	
	osKernelInitialize ();
	LED_Initialize ();
	Init_I2C();
	Init_SPI();
	Init_CAN();
	init_capteur(capteur1,d_11m); 
	init_capteur(capteur2,d_11m);
	
	ID_Tache1 = osThreadCreate ( osThread ( tache1 ), NULL ) ;
	ID_Tache2 = osThreadCreate ( osThread ( tache2 ), NULL ) ;
	ID_Tache2 = osThreadCreate ( osThread ( tache3 ), NULL ) ;
	ID_Tache2 = osThreadCreate ( osThread ( tache4 ), NULL ) ;
	
	osKernelStart();
	osDelay(osWaitForever) ;



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


#include "Driver_USART.h"            // ::CMSIS Driver:USART
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h" 
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "Board_LED.h"
#include "PIN_LPC17xx.h"
#include "RTE_Device.h"                 // Keil::Device:Startup
#include "RTE_Components.h" 
#include "cmsis_os.h"// Component selection








extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART Driver_USART0;
extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;




void initialise_player(void) ;
void delay_ms(int ms) ;
void Init_UART0(void);
void Init_UART1(void) ;
void envoison(uint8_t numFichier);
void initialise_player(void);
void choixPiste(void);
void next1(void);
void pause(void);
	
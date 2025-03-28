#include "LPC17xx.h"                    // Device header
#include "RTE_Components.h"             // Component selection
#include "Driver_USART.h"               // ARM::CMSIS Driver:USART:Custom
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include "RTE_Device.h"                 // Keil::Device:Startup
                                
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"
#include <stdio.h>
#include <string.h>

#define ARM_USART_EVENT_RECEIVE_COMPLETE (1UL << 1)

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

extern ARM_DRIVER_USART Driver_USART1;
ARM_DRIVER_USART *USARTdrv = &Driver_USART1;
char received;
char C[50],a;
char rx_buffer[50];
volatile uint32_t rx_index = 0;
osThreadId tache1_id;

void Init_UART(void);
void UART_Callback(uint32_t event);
void tache1(void const *argument);
osThreadDef(tache1, osPriorityNormal, 1, 0);

int main(void)
{
	osKernelInitialize ();
	
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
	GLCD_DrawString(100,100,"NN");
	Init_UART();
	tache1_id = osThreadCreate(osThread(tache1), NULL);
	
	osKernelStart();
	
	while (1)
  {
  }
}

void Init_UART(void){
	Driver_USART1.Initialize(UART_Callback);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							115200);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1);
}



void UART_Callback(uint32_t event)
{		

		
		if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) 
		{
        osSignalSet(tache1_id, 0x0002);		
    }
    	

}




void tache1(void const *argument){
		uint8_t rx_buffer[50];
	
		// Generation de valeurs fixes

		while(1)
		{
			Driver_USART1.Receive(rx_buffer, sizeof rx_buffer);
			osSignalWait(0x0002, osWaitForever);	
			
			a=rx_buffer[0];
			sprintf(C,"c=%d",a);
			GLCD_ClearScreen();
			GLCD_DrawString(100,100,C);
			osDelay(100);
		}
}
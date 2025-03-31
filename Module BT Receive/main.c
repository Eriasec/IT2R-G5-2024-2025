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

char a;
char rx_buffer[20];
char C,Z,X,Y;


osThreadId ID_Receive;
osThreadId ID_Afficher_nunchuk;

void Receive_UART(void const * argument);
void Afficher_nunchuk(void const * argument);



void Init_UART(void);
void UART_Callback(uint32_t event);

osThreadDef (Receive_UART, osPriorityNormal, 1, 0);
osThreadDef (Afficher_nunchuk, osPriorityNormal, 1, 0);
int main(void)
{
	osKernelInitialize ();
	
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);

	Init_UART();
	
	ID_Receive = osThreadCreate(osThread(Receive_UART), NULL);
	ID_Afficher_nunchuk = osThreadCreate(osThread(Afficher_nunchuk), NULL);
	
	osKernelStart();
	osDelay(osWaitForever) ;
	
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
        osSignalSet(ID_Receive, 0x0002);		
    }
    	

}




void Receive_UART(void const *argument){
		uint8_t rx_buffer[9];

		while(1)
		{
			//osDelay(100);
			
			
			Driver_USART1.Receive(&C, 1);			
			osSignalWait(0x0002, osWaitForever);
			Driver_USART1.Receive(&Z, 1);			
			osSignalWait(0x0002, osWaitForever);
			Driver_USART1.Receive(&X, 1);			
			osSignalWait(0x0002, osWaitForever);
			Driver_USART1.Receive(&Y, 1);			
			osSignalWait(0x0002, osWaitForever);
			
		}
}

void Afficher_nunchuk(void const *argument){
		
char a[50];
char b[50];
		while(1)
		{
		sprintf(a,"C=%d Z=%d ",C,Z);
		sprintf(b,"X=%3d Y=%3d",X,Y);
		GLCD_DrawString(10,10,a);
		GLCD_DrawString(10,50,b);
		
		
		

		
		}
}



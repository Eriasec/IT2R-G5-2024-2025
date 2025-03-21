#include "stm32f4xx.h"                  // Device header
#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "RTE_Components.h"             // Component selection
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include <stdio.h>
#include <string.h>

extern ARM_DRIVER_USART Driver_USART2;

void Init_UART(void);
void UART_Callback(uint32_t event);


int main (void){
	uint8_t tab[1];
	uint8_t tab1[50];
	uint8_t rx_buffer[50]; 
	Init_UART();
	
	
	while (1){
		
		while(Driver_USART2.GetStatus().tx_busy == 1); // attente buffer TX vide
		Driver_USART2.Send(rx_buffer, strlen((char *)rx_buffer));
		osDelay(100);
		Driver_USART2.Receive(tab,12);
		
	}	
	return 0;
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
		uint8_t *buffer;
		int x,y,z;
		x = 1;
		y = 2;
		z = 3;
		
	
		if (event & ARM_USART_EVENT_SEND_COMPLETE) {
        sprintf((char *)buffer, "X:%04d Y:%04d Z:%04d\n", x, y, z);
    }
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
        
    }
	
}



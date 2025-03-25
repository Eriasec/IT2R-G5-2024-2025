#include "LPC17xx.h"                    // Device header
#include "RTE_Components.h"             // Component selection
#include "Driver_USART.h"               // ARM::CMSIS Driver:USART:Custom
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include "RTE_Device.h"                 // Keil::Device:Startup
#include "GPIO.h"                       // Keil.IUT S2/S3::Board Support:GPIO:GPIO

#include <stdio.h>
#include <string.h>

#define ARM_USART_EVENT_RECEIVE_COMPLETE (1UL << 1)

extern ARM_DRIVER_USART Driver_USART1;
ARM_DRIVER_USART *USARTdrv = &Driver_USART1;
char received;
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
	Initialise_GPIO();
	Eteindre_1LED(1);
	Eteindre_1LED(2);
	Eteindre_1LED(3);
	Eteindre_1LED(4);
	Eteindre_1LED(5);
	Eteindre_1LED(6);
	Eteindre_1LED(7);
	Eteindre_1LED(0);
	
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



void UART_Callback(uint32_t event){		
		char msg[50 + 10];
		
		Allumer_1LED(1);
	
		
		if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
        osSignalSet(tache1_id, 0x0002);
				USARTdrv -> Receive(&received,1);
    }
    	
		 if (received == '\n' || received == '\r') {
            rx_buffer[rx_index] = '\0'; // Terminer la chaîne
            rx_index = 0; // Réinitialiser l'index

            // Afficher le message reçu
            
            sprintf(msg, "Reçu : %s\n", rx_buffer);
            USARTdrv->Send(msg, strlen(msg));
        } else {
            // Ajouter au buffer si la limite n'est pas atteinte
            if (rx_index < 50 - 1) {
                rx_buffer[rx_index++] = received;
            }
        }

        // Relancer la réception pour le prochain caractère
        USARTdrv->Receive(&received, 1);
}




void tache1(void const *argument){
		uint8_t rx_buffer[50];
	
		// Generation de valeurs fixes
		int bp = 1;
		while(1){
			sprintf((char *)rx_buffer, " bp: %04d\n", bp);
		
			while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
			Driver_USART1.Receive(rx_buffer, strlen((char *)rx_buffer));
			osSignalWait(0x0002, 1000);	
		}
}
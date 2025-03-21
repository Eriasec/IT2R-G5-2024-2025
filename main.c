#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include <stdio.h>
#include <string.h>
extern ARM_DRIVER_USART Driver_USART3;
extern ARM_DRIVER_USART Driver_USART2;

void Init_UART(void){
	Driver_USART3.Initialize(NULL);        //Recevoir une trame - Broche PD8/9
	Driver_USART3.PowerControl(ARM_POWER_FULL);
	Driver_USART3.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							9600);
	Driver_USART3.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART3.Control(ARM_USART_CONTROL_RX,1);
  
  Driver_USART2.Initialize(NULL);                   //Envoyer la trame traitée pour tester - Broche PA2/3  
	Driver_USART2.PowerControl(ARM_POWER_FULL);
	Driver_USART2.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							9600);
	Driver_USART2.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART2.Control(ARM_USART_CONTROL_RX,1);
}

int main (void){
  int i = 0;
	char trame[500];       // Trame recue
  
  char id[] = "$GPRMC";  // ID du message compris dans la trame
  float  time;           // UTC TIME : hhmmss.sss
  char status;           // "A" si DATA est valide ou "V" si invalide
  float latitude;        // Latitude : ddmm.mmmm
  char NS;               // NORD ou SUD : "N" ou "S"
  float longitude;
  char EW;               // EST ou OUEST : "E" ou "W"
  char debugBuf[20];
  int Nbr1;       // Nombre de champs correctement convertis et affectés
	char Nbr2[3];
  
  
  Init_UART();

	while (1)
  {
		 
		Driver_USART3.Receive(trame,500);       // On recoit la trame du module GPS
    while(Driver_USART3.GetRxCount()<1);

    Nbr1 = sscanf(trame, "$GPRMC,%f,%c,%f,%c,%f,%c", &time, &status, &latitude, &NS, &longitude, &EW);    

    sprintf(debugBuf, "%d\r\n", Nbr1);

    
    while(Driver_USART2.GetStatus().tx_busy == 1);
    Driver_USART2.Send(trame, strlen(trame));



    
    
	}	
	return 0;
}
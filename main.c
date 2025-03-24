#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
#include <stdio.h>
#include <string.h>
extern ARM_DRIVER_USART Driver_USART3;
extern ARM_DRIVER_USART Driver_USART2;
extern ARM_DRIVER_CAN Driver_CAN1;

//void Init_CAN(void){
//  Driver_CAN1.Initialize(NULL, NULL);
//  Driver_CAN1.PowerControl(ARM_POWER_FULL);
//  Driver_CAN1.SetMode(ARM_CAN_MODE_INITIALIZATION);
//  Driver_CAN1.SetBitrate( ARM_CAN_BITRATE_NOMINAL,
//                          125000,
//                          ARM_CAN_BIT_PROP_SEG(5U) |
//                          ARM_CAN_BIT_PHASE_SEG1(1U) |
//                          ARM_CAN_BIT_PHASE_SEG2(1U) |
//                          ARM_CAN_BIT_SJW(1U)
//);
//}

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

	char trame[500];       // Trame recue
  
  char id[] = "$GPRMC";  // ID du message compris dans la trame
  float  time;           // UTC TIME : hhmmss.sss
  char status;           // "A" si DATA est valide ou "V" si invalide
  float latitude;        // Latitude : ddmm.mmmm
  char NS;               // NORD ou SUD : "N" ou "S"
  float longitude;
  char EW;               // EST ou OUEST : "E" ou "W"
  char debugBuf[100];
  int valide;       // Nombre de champs correctement scannés et affectés

  char *p;     //Pointe au début de la séquence "$GPRMC..."
  char msg[] = "Pas de $GPRMC dans ce bloc"; // Message d'erreur
  
  Init_UART();
  

	while (1)
  {
		Driver_USART3.Receive(trame, 500);  // On reçoit la trame du module GPS
    while (Driver_USART3.GetRxCount() < 80);

    p = strstr(trame, "$GPRMC"); // On cherche "$GPRMC", si on trouve, on a l'adresse de la case correspondante

    if (p != NULL) 
    {
      valide = sscanf(p, "$GPRMC,%f,%c,%f,%c,%f,%c",&time, &status, &latitude, &NS, &longitude, &EW);

      sprintf(debugBuf, "valide=%d, lat=%.4f, %c, lon=%.4f, %c, status=%c\r\n", valide, latitude, NS, longitude, EW, status); // On scanne les données
      while (Driver_USART2.GetStatus().tx_busy);
      Driver_USART2.Send(debugBuf, strlen(debugBuf));
    }

    else // Envoi message d'erreur
    {
      while (Driver_USART2.GetStatus().tx_busy);
      Driver_USART2.Send(msg, strlen(msg));
    }
	}	
	return 0;
}
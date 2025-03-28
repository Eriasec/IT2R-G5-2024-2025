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
#include "RFID_tache.h" 




 


	int i ;
  int j ;
  uint8_t trame[50];
  uint8_t uid[4];
  uint8_t UID_cible[4] = {0x30, 0x34, 0x31, 0x42};
  char hexStr[2];
  int hexCount =0 ;
	


void RFID(void *argument) {
	
	while(1) { 
		
		GLCD_DrawString(10, 100, "trame: ");
    GLCD_DrawString(10, 150, "UID: ");
    
    Driver_USART1.Receive(trame,20);
		while(Driver_USART1.GetRxCount()<20);
    for (i = 0; i < 20; i++) {
						sprintf(hexStr, "%02X", trame[i]); // Conversion en hex
            GLCD_DrawString(50 + (i * 18), 100, hexStr); // Affichage des octets en hex
    }
    for (i = 0; i < 16; i++) {
            if (trame[i] == 0x02) {  // Si on trouve 0x02
                for (j = 0; j < 4; j++) {
                    uid[j] = trame[i + 1 + j];  // Capturer les 4 octets suivants
                }
                break;  // Sortir de la boucle dès qu'on a trouvé et extrait l'UID
            }
        }

        for (i = 0; i < 4; i++) {
            // Convertir chaque octet en hex et vérifier s'il contient un chiffre
            sprintf(hexStr, "%02X", uid[i]);
            hexCount += 2;  // Chaque octet hexadécimal est composé de 2 caractères
        }

        // Si nous avons 8 chiffres hexadécimaux (4 octets), afficher l'UID
        if (hexCount == 8) {
            // Afficher l'UID
            for (i = 0; i < 4; i++) {
                sprintf(hexStr, "%02X", uid[i]);
                GLCD_DrawString(50 + (i * 18), 150, hexStr); // Afficher chaque octet de l'UID
            }
            GLCD_DrawString(10, 200, "UID Capturé!");
     
		
    }
    //allumage d'une led si UID trouvé : 
  if (memcmp(uid, UID_cible, 4) == 0) {
    LED_On(5);
    delay_ms(100000); 
    LED_Off(5);
  }

}
	}




















	


	




osThreadId ID_TacheRFID ;
//osThreadDef (RFID, osPriorityNormal, 1, 0); // 1 instance, taille pile par défaut









	

int main (void){
	
	

  Init_UART1();
	Init_UART0();
	LED_Initialize() ;
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_6x8);
	initialise_player();
	
	
	//osKernelInitialize() ;
	
	//ID_TacheRFID = osThreadCreate ( osThread ( RFID ), NULL ) ;
	
	//osKernelStart() ;
	//osDelay(osWaitForever) ;


	
	
	delay_ms(300);

	
	
	while (1){
		
	next();

	delay_ms(100000);
}

}




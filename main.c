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
static uint16_t   background_color = GLCD_COLOR_RED;

osThreadId ID_TacheRFID ;
osThreadId ID_Tachenext ;



 
 



void next(const void *argument){
		
		uint16_t checksum=0 ;
    // Trame complète pour jouer le fichier 1 
    uint8_t next1[10] = {
        0x7E,  // Start
        0xFF,  // Version
        0x06,  // Longueur des données (6 octets)
        0x01,  // choisir un dossier
        0x00,  // Pas de réponse nécessaire
        0x00,  // dossier 1
        0x00,  // fichier 1
        0x00,  // Checksum à calculé
				0x00,
        0xEF   // End
    };
		
		checksum =  ~(next1[1] + next1[2] + next1[3] + next1[4] + next1[5] + next1[6]) + 1;
		
		
		next1[7] = (checksum >> 8) & 0xFF;  // Checksum Poid fort
    next1[8] = checksum & 0xFF;         // Checksum Poid faible
		volume_choix(0x0F);
		
		while(1){
    osSignalWait(0x01,osWaitForever);
    // Envoi de la trame via UART
		
    while (Driver_USART0.GetStatus().tx_busy == 1);  // Attente que la transmission soit libre
    Driver_USART0.Send(next1, 10);  // Envoi des 10 octets
		osDelay(30000);
		pause();
	}
}

void RFID(const void *argument) {
	int i ;
  int j ;
  uint8_t trame[50];
  uint8_t uid[4];
  uint8_t UID_cible[4] = {0x30, 0x34, 0x31, 0x42};
  char hexStr[2];
  int hexCount =0 ;
	
	
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
		osSignalSet(ID_Tachenext,0x01); 
		LED_On(5);
    osDelay(20000);
		
    LED_Off(5);
  }
	else {
		GLCD_SetBackgroundColor(GLCD_COLOR_RED) ;
		GLCD_ClearScreen();
		GLCD_DrawString(10, 200, "Chipeur Arrête de chiper!")   ;
		osDelay(30000);
		GLCD_SetBackgroundColor(GLCD_COLOR_WHITE) ;
		GLCD_ClearScreen();
	}

}
	}

	
osThreadDef (RFID, osPriorityNormal, 1, 0); // 1 instance, taille pile par défaut
osThreadDef (next, osPriorityNormal, 1, 0); // 1 instance, taille pile par défaut








	

int main (void){
	osKernelInitialize() ;
	
  Init_UART1();
	Init_UART0();
	LED_Initialize() ;
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_6x8);
	initialise_player();
	
	
	
	ID_TacheRFID = osThreadCreate ( osThread ( RFID ), NULL ) ;
	ID_Tachenext = osThreadCreate ( osThread ( next ), NULL ) ;
	
	osKernelStart() ;
	
	osDelay(osWaitForever) ;

	while (1){
		osDelay(osWaitForever);
	}
}

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

extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART Driver_USART0;
extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;
char numfichierHexa[3]; 
char datadfplayer[10];

void initialise_player(void) {
    // Trame d'initialisation pour U-disk (ID 0x01)
    uint8_t initCommand[] = {
        0x7E,  // Début de la commande
        0xFF,  // Information sur la version du module
        0x06,  // Longueur des données (6 octets)
        0x3F,  // Type d'appareil (ici U-disk)
        0x00,  // Paramètre supplémentaire
        0x00,  // Identifiant du périphérique
        0x01,  // Identifiant spécifique (U-disk)
        0xDD,  // Checksum calculé (à remplacer selon les besoins)
        0xEF   // Fin de la commande
    };

    // Envoi de la trame via UART (s'assurer que l'UART est prêt)
    while (Driver_USART0.GetStatus().tx_busy == 1);  // Attente que l'UART soit prêt
    Driver_USART0.Send(initCommand, sizeof(initCommand));  // Envoi de la trame
	}

void delay_ms(int ms) {
    int i;
    while(ms > 0) {
        i = 0;
        while(i < 1000) {  // Ajuster cette valeur selon la fréquence d'horloge de ton MCU
            i++;
        }
        ms--;
    }
}
void Init_UART1(void){
	Driver_USART1.Initialize(NULL);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							9600);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1);

}
void Init_UART0(void){
	Driver_USART0.Initialize(NULL);
	Driver_USART0.PowerControl(ARM_POWER_FULL);
	Driver_USART0.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							9600);
	Driver_USART0.Control(ARM_USART_CONTROL_TX,1);
	
}


	

void envoison(void){
	
    // Trame complète pour jouer le fichier 19 (0x13)
    uint8_t datadfplayer[10] = {
        0x7E,  // Start
        0xFF,  // Version
        0x06,  // Longueur des données (6 octets)
        0x03,  // Commande pour jouer un fichier
        0x00,  // Pas de réponse nécessaire
        0x00,  // Octet élevé du numéro du fichier (haut de 19 = 0x00)
        0x13,  // Octet faible du numéro du fichier (bas de 19 = 0x13)
        0xFF,  // Byte checksum (calculé)
        0xE6,  // Byte checksum (calculé)
        0xEF   // End
    };
    
    // Envoi de la trame via UART
    while (Driver_USART0.GetStatus().tx_busy == 1);  // Attente que la transmission soit libre
    Driver_USART0.Send(datadfplayer, 10);  // Envoi des 10 octets
}
	
	

int main (void){
	int i ;
  int j ;
  uint8_t trame[50];
  uint8_t uid[4];
  uint8_t UID_cible[4] = {0x30, 0x34, 0x31, 0x42};
  char hexStr[2];
  int hexCount =0 ;
	
  
  Init_UART1();
	Init_UART0();
	LED_Initialize() ;
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_6x8);
	initialise_player();

	//envoie_ddfplayer(19);
	while (1){
		//envoison();
		
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
    delay_ms(1000000); 
    LED_Off(5);
  }
		
	
} return 0;}


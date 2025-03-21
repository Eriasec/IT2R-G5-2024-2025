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



	
	char numfichierHexa[3]; 
	char datadfplayer[10];

	
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

void delay_ms(int ms) {
    int i;
    while(ms > 0) {
        i = 0;
        while(i < 1000) {  // Ajuster cette valeur selon la fréquence d'horloge 
            i++;
        }
        ms--;
    }
}

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
		



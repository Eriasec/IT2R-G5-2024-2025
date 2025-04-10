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
 
extern osThreadId ID_TacheRFID ;
extern osThreadId  ID_TacheJouerSon ;


void volume_choix(uint8_t choix){
	
		uint16_t checksum=0 ;
    // Trame complète pour jouer le fichier 1 
    uint8_t volume[10] = {
        0x7E,  // Start
        0xFF,  // Version
        0x06,  // Longueur des données (6 octets)
        0x06,  // cmd
        0x00,  // Pas de réponse nécessaire
        0x00,  // volume 1
        0x00,  // volume2
        0x00,  // Checksum à calculé
				0x00,
        0xEF   // End
    };
		volume[6]=choix;
		checksum =  ~(volume[1] + volume[2] + volume[3] + volume[4] + volume[5] + volume[6]) + 1;
		
		
		volume[7] = (checksum >> 8) & 0xFF;  // Checksum Poid fort
    volume[8] = checksum & 0xFF;         // Checksum Poid faible
    
    // Envoi de la trame via UART
    while (Driver_USART0.GetStatus().tx_busy == 1);  // Attente que la transmission soit libre
    Driver_USART0.Send(volume, 10);  // Envoi des 10 octets
	
	}

 
 
 

void pause(void){
	
		uint16_t checksum=0 ;
    // Trame complète pour jouer le fichier 1 
    uint8_t pause[10] = {
        0x7E,  // Start
        0xFF,  // Version
        0x06,  // Longueur des données (6 octets)
        0x0E,  // cmd
        0x00,  // Pas de réponse nécessaire
        0x00,  // dossier 1
        0x00,  // fichier 1
        0x00,  // Checksum à calculé
				0x00,
        0xEF   // End
    };
		checksum =  ~(pause[1] + pause[2] + pause[3] + pause[4] + pause[5] + pause[6]) + 1;
		
		
		pause[7] = (checksum >> 8) & 0xFF;  // Checksum Poid fort
    pause[8] = checksum & 0xFF;         // Checksum Poid faible
    
    // Envoi de la trame via UART
    while (Driver_USART0.GetStatus().tx_busy == 1);  // Attente que la transmission soit libre
    Driver_USART0.Send(pause, 10);  // Envoi des 10 octets
	
	}




	

	
	
	
	
	
void choixPiste(void){
	
		uint16_t checksum=0 ;
    // Trame complète pour jouer le fichier 1 
    uint8_t choix[10] = {
        0x7E,  // Start
        0xFF,  // Version
        0x06,  // Longueur des données (6 octets)
        0x0F,  // choisir un dossier
        0x00,  // Pas de réponse nécessaire
        0x01,  // dossier 1
        0x01,  // fichier 1
        0x00,  // Checksum à calculé
				0x00,
        0xEF   // End
    };
		checksum =  ~(choix[1] + choix[2] + choix[3] + choix[4] + choix[5] + choix[6]) + 1;
		
		
		choix[7] = (checksum >> 8) & 0xFF;  // Checksum Poid fort
    choix[8] = checksum & 0xFF;         // Checksum Poid faible
    
    // Envoi de la trame via UART
    while (Driver_USART0.GetStatus().tx_busy == 1);  // Attente que la transmission soit libre
    Driver_USART0.Send(choix, 10);  // Envoi des 10 octets
	
	
}
	
void envoison(uint8_t numFichier){

		uint16_t checksum=0 ;
    // Trame complète pour jouer le fichier 1 
    uint8_t datadfplayer[10] = {
        0x7E,  // Start
        0xFF,  // Version
        0x06,  // Longueur des données (6 octets)
        0x0F,  // Commande pour jouer un fichier
        0x00,  // Pas de réponse nécessaire
        0x01,  // Octet élevé du numéro du Dossier
        0x01,  // Octet faible du numéro du fichier 
        0x00,  // Checksum à calculé
				0x00,
        0xEF   // End
    };
		datadfplayer[6]=numFichier ;
		checksum =  ~(datadfplayer[1] + datadfplayer[2] + datadfplayer[3] + datadfplayer[4] + datadfplayer[5] + datadfplayer[6]) + 1;
		
		datadfplayer[7] = (checksum >> 8) & 0xFF;  // Checksum Poid fort
    datadfplayer[8] = checksum & 0xFF;         // Checksum Poid faible
    
    // Envoi de la trame via UART
    while (Driver_USART0.GetStatus().tx_busy == 1);  // Attente que la transmission soit libre
    Driver_USART0.Send(datadfplayer, 10);  // Envoi des 10 octets
}
void Tx_hautparleur_cb( uint32_t ev){
	
	if((ev & ARM_USART_EVENT_TX_COMPLETE )==ARM_USART_EVENT_TX_COMPLETE)
	{
		osSignalSet(ID_TacheJouerSon,0x02);
	}
	
	
	
	
}




void Rx_RFID_cb(uint32_t ev)
{
	if((ev & ARM_USART_EVENT_RECEIVE_COMPLETE )==ARM_USART_EVENT_RECEIVE_COMPLETE)
	{
		osSignalSet(ID_TacheRFID,0x01);
	}

}
	
void Init_UART0(void){
	Driver_USART0.Initialize(Rx_RFID_cb);
	Driver_USART0.PowerControl(ARM_POWER_FULL);
	Driver_USART0.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							9600);
	Driver_USART0.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART0.Control(ARM_USART_CONTROL_RX,1);
	
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
        while(i < 16000) {  // Ajuster cette valeur selon la fréquence d'horloge 
            i++;
        }
        ms--;
    }
}

//pas touche a l'init 
void initialise_player(void) {
   uint16_t checksum=0 ;
	
    uint8_t initCommand[10] = {
        0x7E,  // Début de la commande
        0xFF,  // Information sur la version du module
        0x06,  // Longueur des données (6 octets)
        0x3F,  // commande
				0x00,  // feedback
        0x00,  // Paramètre 1
        0x02,  // Paramètre 2
        0x00,  // Checksum à calculé
				0x00,
				0xEF   // Fin de la commande
    };
		checksum =  ~(initCommand[1] + initCommand[2] + initCommand[3] + initCommand[4] + initCommand[5] + initCommand[6]) + 1;
		
		initCommand[7] = (checksum >> 8)  &0xFF;  // Checksum Poid fort
    initCommand[8] = checksum & 0xFF;         // Checksum Poid faible
		
			
    // Envoi de la trame via UART (s'assurer que l'UART est prêt)
    while (Driver_USART0.GetStatus().tx_busy == 1);  // Attente que l'UART soit prêt
    Driver_USART0.Send(initCommand, 10);  // Envoi de la trame
	}
		



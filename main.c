
// Résolution ecran LCD 320x240

#include "UART_LPC17xx.h"
#include "stdio.h"
#include "math.h"
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "GLCD_Fonts.h"                 // Keil.MCB1700::Board Support:Graphic LCD
#include "cmsis_os.h"

// __________ Dénifintion des octets utilisés __________ \\

#define LIDAR_START_HEADER 					0xA5
#define LIDAR_RESPONSE_HEADER				0x5A
#define LIDAR_REQUEST_RESET 				0x40
#define LIDAR_REQUEST_STOP 					0x25
#define LIDAR_REQUEST_SCAN					0x20
#define LIDAR_REQUEST_FORCE_SCAN		0x21
#define LIDAR_REQUEST_GET_INFO			0x50

extern GLCD_FONT GLCD_Font_16x24;

// __________ Prototypes Fonctions UART __________ 
void Init_UART1(void);

// __________ Prototypes Fonctions PWM __________ 
void InitPWM(int alpha);
void InitTimer(int prescaler, int match);

// __________ Prototypes Fonctions LIDAR __________ 
void LIDAR_Reset(void);
void LIDAR_Stop(void);
void LIDAR_Get_Info(void);		// Récupère les infos du lidar **Pour on ne lit pas les données renvoyées**
void LIDAR_Scan(void);
void LIDAR_Force_Scan(void);
void LIDAR_Attente_Header(void);

// __________ Prototypes Callback __________ \\

void callbackUSART(uint32_t event);

// __________ OS Thread ID __________ \\

osThreadId ID_ThreadLidarUART;
osThreadId ID_ThreadLidarTraitement;
osThreadId ID_ThreadClearScreen;

// __________ Variables globales __________ \\

typedef struct {
	uint8_t reception[200];
} maStructure;

// __________ OS MailBox __________ \\

osMailQId ID_BAL;
osMailQDef(BAL_Reception, 5, maStructure);

// __________ OS MailBox __________ \\

osMutexId ID_mut_GLCD;
osMutexDef(mut_GLCD);



// __________ OS Threads __________ \\

void threadLidarUART(void const * argument) {
	maStructure *t_ptr;
	
	uint8_t reception[200] = {0};
	char state = 0;
	
	ID_BAL = osMailCreate(osMailQ(BAL_Reception),NULL);
	
	GLCD_DrawChar(0,24,'e');
	LIDAR_Scan();
	osSignalWait(0x02, osWaitForever); // _____ Attente de la fin du send _____
	
	while(1) {
		switch(state) {
			case 0:
				Driver_USART1.Receive(reception, 7);		// _____ Reception du header _____
				osSignalWait(0x01, osWaitForever);			// _____ Attente de la fin du receive _____
				if((reception[0] == LIDAR_START_HEADER) && (reception[1] == LIDAR_RESPONSE_HEADER)) {		// _____ Vérification du header (0xA5, 0x5A)
					state = 1;
				}
				break;
				
			case 1:
				t_ptr = osMailAlloc(ID_BAL,osWaitForever);
				Driver_USART1.Receive(t_ptr->reception, 200);
				osSignalWait(0x01, osWaitForever);
				osMailPut(ID_BAL, t_ptr);
				break;
		}
	}
}

void threadLidarTraitement(void const * agument) {
	// Declaration des variables RTOS
	maStructure *r_ptr;				// Pointeur vers la boite mail
	osEvent evt;							// Event
	
	int i;
//	float angleMax = 0, angleMin = 360;
	uint16_t angle, distance, angleVeritable, distanceVeritable, x=0, y=0;
//	char tab[10];
	
	while(1) {
		evt = osMailGet(ID_BAL,osWaitForever);
		if(evt.status == osEventMail) {
			r_ptr = evt.value.p;
			for(i=0; i<200; i+=5) {
				if(r_ptr->reception[i] == 0x3E) {
					angle = 		(r_ptr->reception[i+1] & 0xFE	) >> 1;		// _____ Octets 0 a  6 de l'angle _____
					angle |= 		(r_ptr->reception[i+2]				) << 7;		// _____ Octets 7 a 14 de l'angle _____
					distance = 	(r_ptr->reception[i+3]				) << 0;		// _____ Octets 0 a  7 de l'angle _____
					distance |= (r_ptr->reception[i+4]				) << 8;		// _____ Octets 8 a 15 de l'angle _____
					angleVeritable = angle >> 6;												// Division par 64 pour obtenir le vrai angle
					distanceVeritable = (distance >> 2) - 100;					// Division par 4 pour obtenir la vraie distance et - 100 car a moins de 10cm la qualite est trop basse
					distanceVeritable = distanceVeritable >> 2;					// Mise a l'echelle
					x = (cos(((double) angleVeritable)*3.1415/180) * distanceVeritable) + 160;				// calcul de la composante en x (+160 pour afficher au milieu)
					y = (sin(((double) angleVeritable)*3.1415/180) * distanceVeritable) + 120;				// calcul de la composante en y (+120 pour afficher au milieu)
					if(y > 240) {
						y = 240;
					}
					if(x > 320) {
						x = 320;
					}
					
					osMutexWait(ID_mut_GLCD, osWaitForever);
					GLCD_DrawPixel(x,y);
					osMutexRelease(ID_mut_GLCD);
				}
			}
			osMailFree(ID_BAL, r_ptr);
		}
	}
}

void threadClearScreen(void const * argument) {
	int i, j;
	while(1) {
		osDelay(10000);
		osMutexWait(ID_mut_GLCD, osWaitForever);
		GLCD_ClearScreen();
		for(i=0; i<3; i++) {
			for(j=0; j<3; j++) {
				GLCD_DrawPixel(i+159, j+119);
			}
		}
		osMutexRelease(ID_mut_GLCD);
	}
}


// __________ OS Thread Def __________ \\

osThreadDef(threadLidarUART, osPriorityRealtime, 1, 0);		
osThreadDef(threadLidarTraitement, osPriorityNormal, 1, 0);	 
osThreadDef(threadClearScreen, osPriorityAboveNormal, 1, 0);






// __________ Main __________ \\

int main(void) {
	int i,j;
	// _____ Initialisation des threads _____
	osKernelInitialize();
	ID_ThreadLidarUART = osThreadCreate(osThread(threadLidarUART), NULL);
	ID_ThreadLidarTraitement = osThreadCreate(osThread(threadLidarTraitement), NULL);
	ID_ThreadClearScreen = osThreadCreate(osThread(threadClearScreen), NULL);
	
	ID_mut_GLCD = osMutexCreate(osMutex(mut_GLCD));
	
	// _____ Initialisation de l'UART _____
	Init_UART1();
	InitPWM(600);
//	InitTimer(9999,12499);
	
	// _____ Initialisation du GLCD _____
	GLCD_Initialize();
	GLCD_SetFont(&GLCD_Font_16x24);
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
	GLCD_ClearScreen();
	for(i=0; i<3; i++) {
		for(j=0; j<3; j++) {
			GLCD_DrawPixel(i+159, j+119);
		}
	}
	
	// _____ Lancement RTOS _____
	osKernelStart();
	
	osDelay(osWaitForever);
	
	return 0;
}





// __________ Fonctions UART __________ \\

void Init_UART1(void) {
	Driver_USART1.Initialize(callbackUSART);								// Initialisation avec callback
	Driver_USART1.PowerControl(ARM_POWER_FULL);							// Allimentation par la carte
	Driver_USART1.Control(	ARM_USART_MODE_ASYNCHRONOUS |		// Mode asynchrone
													ARM_USART_DATA_BITS_8 |					// 8 bits de donnée
													ARM_USART_STOP_BITS_1 |					// 1 bit de stop
													ARM_USART_PARITY_NONE |					// Pas de parité
													ARM_USART_FLOW_CONTROL_NONE,		// Pas de contrôle de flux
													115200);												// 115200 bauds
	Driver_USART1.Control(ARM_USART_CONTROL_TX, 1);					// Activation TX
	Driver_USART1.Control(ARM_USART_CONTROL_RX, 1);					// Activation RX
}

// __________ Fonctions PWM __________ \\

void InitPWM(int alpha) {
		// Utilisation d la PWM1.2 
		// MR2 rapport cyclique alpha du moteur
		// ils varient de 0 à 99 (cas MR0 associé à 99)
		// La fréquence de la PWM vaut 10KHz soit 100 us
		// la fréquence de base est donnée par l'horloge interne de fréquence 25 MHz soir 40ns

		LPC_SC->PCONP = LPC_SC->PCONP | 0x00000040;   // enable PWM1

		// calcul des diviseurs
		// prescaler+1 = 1us/40ns = 25 cela donne une horloge de base de fréquence 10 KHz = 100 us
		// valeur + 1 = 100 cela fait 1ms 
		// la duree de comptage vaut 1 us

		LPC_PWM1->PR = 0;  // prescaler
		LPC_PWM1->MR0 = 999;    // MR0+1=100   la période de la PWM vaut 100µs

		LPC_PINCON->PINSEL7 = LPC_PINCON->PINSEL7| (3 << 18); // P3.25 est la sortie 1 PWM1   bit19 & bit18 

																	
		LPC_PWM1->MCR = LPC_PWM1->MCR | 0x00000002; // Compteur relancé quand MR0 repasse à 0
		LPC_PWM1->LER = LPC_PWM1->LER | 0x0000000F;  // ceci donne le droit de modifier dynamiquement la valeur du rapport cyclique
		// bit 0 = MR0    bit 1 MR1 bit2 MR2 bit3 = MR3
		LPC_PWM1->PCR = LPC_PWM1->PCR | 0x00000e00;  // autorise les sortie PWM1/2/3 bits 9, 10, 11


		LPC_PWM1->MR2 = alpha;							//Rapport cyclique alpha OU vitesse

		LPC_PWM1->TCR = 1;  /*validation de timer  et reset counter */
}

// __________ Fonctions TIMER (pour clearscreen) __________ \\

//void InitTimer(int prescaler, int match) {	//Configuration du TIMER0
//  LPC_TIM0->PR = prescaler;                	// Prescaler PR
//  LPC_TIM0->MR0 = match;                   	// valeur de MR
//  LPC_TIM0->MCR = LPC_TIM0->MCR | (3<<0); 	// RAZ du compteur + interruption
//  LPC_TIM0->TCR = 1 ;                     	// Lancement Timer
//  
//  //Configuration Interuption sur TIMER0
//  NVIC_SetPriority(TIMER0_IRQn,0);        	// TIMER0 (IRQ1) : interruption de priorité 0
//  NVIC_EnableIRQ(TIMER0_IRQn);            	// active les interruptions TIMER0
//}

//void TIMER1_IRQHandler(void) { 			// Fonction d'interruption sur TIMER1
//  LPC_TIM1->IR = (1<<0); 				//baisse le drapeau dû à MR0
//	osSignalSet(ID_ThreadClearScreen, 0x0001);
//}

// __________ Fonctions LIDAR __________ \\

void LIDAR_Get_Info(void) {
	char tab[2] = {LIDAR_START_HEADER,LIDAR_REQUEST_GET_INFO};
	
	while(Driver_USART1.GetStatus().tx_busy);
	Driver_USART1.Send(tab,2);
}

void LIDAR_Scan(void) {
	char tab[2] = {LIDAR_START_HEADER,LIDAR_REQUEST_SCAN};
	
	while(Driver_USART1.GetStatus().tx_busy);
	Driver_USART1.Send(tab,2);
}

void LIDAR_Reset(void) {
	char tab[2] = {LIDAR_START_HEADER,LIDAR_REQUEST_RESET};
	
	while(Driver_USART1.GetStatus().tx_busy);
	Driver_USART1.Send(tab,2);
}

void LIDAR_Stop(void) {
	char tab[2] = {LIDAR_START_HEADER,LIDAR_REQUEST_STOP};
	
	while(Driver_USART1.GetStatus().tx_busy);
	Driver_USART1.Send(tab,2);
}

void LIDAR_Force_Scan(void) {
	char tab[2] = {LIDAR_START_HEADER,LIDAR_REQUEST_FORCE_SCAN};
	
	while(Driver_USART1.GetStatus().tx_busy);
	Driver_USART1.Send(tab,2);
}

// __________ Fonctions Callback __________ \\

void callbackUSART(uint32_t event) {
	if(event & ARM_USART_EVENT_RECEIVE_COMPLETE) {		// Check si réveil sur fin de reception
		osSignalSet(ID_ThreadLidarUART, 0x01);				// Réveil de la tache UART sur évenement 0x01
	}
	if(event & ARM_USART_EVENT_SEND_COMPLETE) {				// Check si réveil sur fin d'envoi
		osSignalSet(ID_ThreadLidarUART, 0x02);				// Réveil de la tache UART sur évenement 0x01
	}
}
